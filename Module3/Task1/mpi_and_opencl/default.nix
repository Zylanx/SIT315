#! /usr/bin/env nix-shell
#! nix-shell --command "nix-build; mpirun -np 3 ./result/bin/mpi_and_opencl"
{ pkgs ? import <nixpkgs> { }, stdenv ? pkgs.stdenv, ... }:
stdenv.mkDerivation {
    name = "mpi_and_opencl";

    dontStrip = true;

    nativeBuildInputs = with pkgs; [
        cmake
        mpi
        opencl-headers
        opencl-clhpp
        ocl-icd
    ];

    src = ./.;
}
