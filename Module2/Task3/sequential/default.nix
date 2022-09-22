{ pkgs ? import <nixpkgs> { }, stdenv ? pkgs.stdenv, ... }:
stdenv.mkDerivation {
    name = "TrafficControl";

    nativeBuildInputs = with pkgs; [
        cmake
        #howard-hinnant-date
        howard-hinnant-date.dev
    ];

    src = ./.;
}
