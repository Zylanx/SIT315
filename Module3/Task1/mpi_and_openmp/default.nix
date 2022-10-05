#! /usr/bin/env nix-shell
#! nix-shell --command "nix-build; mpirun -np 4 ./result/bin/mpi_and_openmp"
{ pkgs ? import <nixpkgs> { }, stdenv ? pkgs.stdenv, ... }:
stdenv.mkDerivation {
    name = "mpi_and_openmp";

    nativeBuildInputs = with pkgs; [
        cmake
        mpi
    ];

    src = ./.;
}
