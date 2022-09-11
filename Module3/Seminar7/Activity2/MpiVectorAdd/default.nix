{stdenv, cmake, mpi}:
stdenv.mkDerivation {
    name = "MpiVectorAdd";

    nativeBuildInputs = [ cmake mpi ];

    src = ./.;
}
