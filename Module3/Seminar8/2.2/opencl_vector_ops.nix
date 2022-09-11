{stdenv, opencl-headers, opencl-clhpp, ocl-icd}:
stdenv.mkDerivation {
    name = "opencl_vector_ops";

    buildInputs = [
        opencl-headers
        opencl-clhpp
        ocl-icd
    ];

    src = ./.;

    dontConfigure = true;

    buildPhase = ''
        g++ -lOpenCL -o vector_ops vector_ops.cpp
    '';

    installPhase = ''
        install -Dm755 vector_ops $out/bin/vector_ops
        install -Dm777 vector_ops.cl $out/bin/vector_ops.cl
    '';
}
