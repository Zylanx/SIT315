#! /usr/bin/env nix-shell
#! nix-shell --command "nix-build; mpirun -np 4 ./result/bin/mpi_only"
{ pkgs ? import <nixpkgs> { }, stdenv ? pkgs.stdenv, ... }:
stdenv.mkDerivation {
    name = "mpi_only";

    nativeBuildInputs = with pkgs; [
        cmake
        mpi
    ];

    src = ./.;
}
