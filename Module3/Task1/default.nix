let
    # Import the overall nixpkgs listing and our mpi_only derivation
    pkgs = import <nixpkgs> {};
    mpi_only = pkgs.callPackage ./mpi_only {};

    # The number of cores to give each VM
    perVMCores = 4;
    # The number of processes to start on each node
    processesPerHost = perVMCores;


    # Generate the arguments string to be passed to MPI.
    # We use a let expression to define all of the changeable options, then process them further into the
    # arguments to pass, then combine them into a string.
    mpirunArguments = let
        inherit (builtins) length concatStringsSep;

        process = "${mpi_only}/bin/mpi_only";  # The process that will be distributed
        hostNames = [  # The names of the hosts to start the processes on
            "master"
            "head"
        ];

        totalProcesses = (length hostNames) * processesPerHost;
        hosts = concatStringsSep "," (map (host: host + ":" + toString processesPerHost) hostNames);  # Processes the hostnames into a --hosts argument

        # Workaround to get SSH in the VMs working.
        # OpenMPI needs SSH to have already added the host to the known_hosts.
        # It can't press "y" to add it when asked. Annoying.
        sshOption = "--mca plm_rsh_agent 'ssh -q -o StrictHostKeyChecking=no'";

        # Workaround for OpenMPI issues regarding network interfaces
        # I think it is to do with when multiple interfaces can reach?
        # https://github.com/open-mpi/ompi/issues/6240
        ethOption = "--mca btl_tcp_if_exclude eth0,127.0.0.0/8";

        # If there are more processses than cores assigned, we need to enable oversubscribing
        oversubscribe = if processesPerHost > perVMCores then "--oversubscribe" else "";
    in
         "--allow-run-as-root ${sshOption} ${ethOption} ${oversubscribe} --host ${hosts} -np ${toString totalProcesses} ${process}";

    # These are options that are common to the nodes.
    sharedConfig = {
        # Tells QEMU that graphics is not enabled
        virtualisation.graphics = false;

        # Sets the cores for each VM to the configured number
        virtualisation.cores = perVMCores;

        # Disables the firewall to make sure OpenMPI can call back. There should be a better way
        # But these are closed off VMs so security doesn't matter.
        networking.firewall.enable = false;
    };

# Finally we define the NixOS test apparatus.
in pkgs.nixosTest ({
    name = "mpi_only_vms";  # The name of the test, doesn't really matter.

    # Our nodes included in the test. We will have the master node, and a head node.
    # The master is able to start OpenMPI processes on both itself, and head.
    nodes = {
        master = {config, pkgs, ...}: {
            imports = [ sharedConfig ];
        };

        head = {config, pkgs, ...}: {
            imports = [ sharedConfig ];

            # Enable SSHD on head so that OpenMPI can connect to it.
            services.openssh = {
                enable = true;
            };
        };
    };

    # The test script to run on the nodes.
    # We are doing a very simple test with OpenMPI, so all we need to do is start the VMs,
    # generate an SSH key for the master to connect to head with,
    # then start the test. As simple as that.
    testScript = ''
        start_all()

        head.wait_for_unit("sshd")
        master.wait_for_unit("network.target")

        with subtest("Generate Keys"):
            # Generate a shared key between them
            master.succeed("mkdir -m 700 /root/.ssh")
            master.succeed('${pkgs.openssh}/bin/ssh-keygen -t ed25519 -f /root/.ssh/id_ed25519 -N ""')
            public_key = master.succeed("${pkgs.openssh}/bin/ssh-keygen -y -f /root/.ssh/id_ed25519")
            public_key = public_key.strip()
            master.succeed("chmod 600 /root/.ssh/id_ed25519")

            # Setup that key in the head
            head.succeed("mkdir -m 700 /root/.ssh")
            head.succeed("echo '{}' > /root/.ssh/authorized_keys".format(public_key))

        with subtest("MPI Run"):
            print("\n\n\033[0;34mRunning MPI\033[0m\n\n")
            results = master.execute("${pkgs.mpi}/bin/mpirun ${mpirunArguments}")
            print("\n\n\n\033[0;34mMPI Results:\033[0m")
            print(results[1])
            print("\n\n")
    '';
})
