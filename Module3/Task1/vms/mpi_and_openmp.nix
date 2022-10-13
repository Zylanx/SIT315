let
    # Import the overall nixpkgs listing and our mpi_only derivation
    pkgs = import <nixpkgs> {};

    program = rec { name = "mpi_and_openmp"; package = pkgs.callPackage (../. + "/${name}") {}; };

    # The number of total processes to use
    totalProcesses = 4;
    # The number of hosts (including master) to start
    numberOfHosts = 4;
    # The number of runs
    numberOfRuns = 5;
    # Number of runs before recording
    numberOfPreruns = 3;
    # The number of cores to give each VM
    perVMCores = 2;
    # The number of processes to start on each node
    processesPerHost = perVMCores;

    hostNames = let
        inherit (builtins) genList;
    in genList (x: "head" + (toString (x + 1))) (numberOfHosts - 1);

    # Generate the arguments string to be passed to MPI.
    # We use a let expression to define all of the changeable options, then process them further into the
    # arguments to pass, then combine them into a string.
    mpirunArguments = let
        inherit (builtins) length concatStringsSep;

        process = "${program.package}/bin/${program.name}";  # The process that will be distributed

        hostNamesInternal = [  # The names of the hosts to start the processes on
            "master"
        ] ++ hostNames;

        #totalProcesses = (length hostNamesInternal) * processesPerHost;
        hosts = concatStringsSep "," (map (host: host + ":" + toString processesPerHost) hostNamesInternal);  # Processes the hostnames into a --hosts argument

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
    name = "${program.name}_vms";  # The name of the test, doesn't really matter.

    # Our nodes included in the test. We will have the master node, and a head node.
    # The master is able to start OpenMPI processes on both itself, and head.
    nodes = let
        master = {config, pkgs, ...}: {
            imports = [ sharedConfig ];
        };

        node_template = {config, pkgs, ...}: {
            imports = [ sharedConfig ];

            # Enable SSHD on head so that OpenMPI can connect to it.
            services.openssh = {
                enable = true;
            };
        };

        node_set = let
            inherit (builtins) listToAttrs map;
        in
            listToAttrs ((map (x: { name = x; value = node_template; }) hostNames) ++ [ { name = "master"; value = node_template; } ]);
    in
        node_set;

    # The test script to run on the nodes.
    # We are doing a very simple test with OpenMPI, so all we need to do is start the VMs,
    # generate an SSH key for the master to connect to head with,
    # then start the test. As simple as that.
    testScript = let
        template_wait_string = hostName: "${hostName}.wait_for_unit(\"sshd\")\n";

        hostWaitStrings = let
            inherit (builtins) foldl' traceVal;
        in
            foldl' (x: y: x + (template_wait_string y)) "" hostNames;

        template_ssh_string = hostName: "    ${hostName}.succeed(\"mkdir -m 700 /root/.ssh\")\n    ${hostName}.succeed(\"echo '{}' > /root/.ssh/authorized_keys\".format(public_key))\n";

        hostSSHStrings = let
            inherit (builtins) foldl' traceVal;
        in
            foldl' (x: y: x + (template_ssh_string y)) "" hostNames;

    in
        pkgs.lib.debug.traceVal ''
            start_all()

        '' + hostWaitStrings + ''
            master.wait_for_unit("network.target")

            with subtest("Generate Keys"):
                # Generate a shared key between them
                master.succeed("mkdir -m 700 /root/.ssh")
                master.succeed('${pkgs.openssh}/bin/ssh-keygen -t ed25519 -f /root/.ssh/id_ed25519 -N ""')
                public_key = master.succeed("${pkgs.openssh}/bin/ssh-keygen -y -f /root/.ssh/id_ed25519")
                public_key = public_key.strip()
                master.succeed("chmod 600 /root/.ssh/id_ed25519")

                # Setup that key in the head
                #head.succeed("mkdir -m 700 /root/.ssh")
                #head.succeed("echo '{}' > /root/.ssh/authorized_keys".format(public_key))
        '' + hostSSHStrings + ''

            with subtest("MPI Run"):
                mpi_results = []

                print("\n\n\033[0;34mRunning MPI Preruns\033[0m\n\n")
                for _ in range(${toString numberOfPreruns}):
                    master.execute("${pkgs.mpi}/bin/mpirun ${mpirunArguments}")

                print("\n\n\033[0;34mRunning MPI\033[0m\n\n")
                for x in range(${toString numberOfRuns}):
                    results = master.execute("${pkgs.mpi}/bin/mpirun ${mpirunArguments}")
                    print("\n\n\n\033[0;34mMPI Results:\033[0m")
                    print(results[1])
                    print("\n\n")
                    mpi_results.append((x, results[1]))

                from datetime import datetime
                import os
                now = datetime.now()

                buffer = "==== RUN: " + now.strftime("%Y/%m/%d %H:%M:%S") + " ====\n"
                for (run, result) in mpi_results:
                    buffer += "run: " + str(run + 1) + "\n"
                    buffer += result + "\n\n"

                print(buffer)

                with open(str(os.getenv('out')) + "/results.txt", "w") as f:
                    f.write(buffer)
        '';
})
