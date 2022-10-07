#! /usr/bin/env nix-shell
#! nix-shell --command "nix run --impure --expr \"import ./. {}\""
{ pkgs ? import <nixpkgs> { }, stdenv ? pkgs.stdenv, ... }:
stdenv.mkDerivation {
    name = "TrafficControl";

    nativeBuildInputs = with pkgs; [
        cmake
        pkg-config
    ];

    buildInputs = with pkgs; [
        howard-hinnant-date.dev
        boost
    ];

    src = ./.;

    meta.mainProgram = "multithreaded";
}
