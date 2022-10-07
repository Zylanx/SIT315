#! /usr/bin/env nix-shell
#! nix-shell --command "nix run --impure --expr \"import ./. {}\""
{ pkgs ? import <nixpkgs> { }, stdenv ? pkgs.stdenv, ... }:
stdenv.mkDerivation {
    name = "TrafficControl";

    nativeBuildInputs = with pkgs; [
        cmake
        #howard-hinnant-date
        howard-hinnant-date.dev
    ];

    src = ./.;

    meta.mainProgram = "sequential";
}
