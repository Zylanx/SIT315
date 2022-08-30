{stdenv, cmake, mpi}:
stdenv.mkDerivation {
    name = "hello_mpi";

    nativeBuildInputs = [ cmake mpi ];

    src = ./.;
}
