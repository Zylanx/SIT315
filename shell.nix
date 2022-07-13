{ pkgs ? import <nixpkgs> { config.allowUnfree = true; } }:
let
  buildPackages = with pkgs; [
    coreutils
    binutils
    glibc
    gcc
    cmake
    ninja
    zlib
    bash
  ];
in
  (pkgs.buildFHSUserEnv {
    name = "cpp-env";

    targetPkgs = pkgs: (with pkgs; [
      platformio
      pkg-config
      jetbrains.clion
    ] ++ buildPackages);

    runScript = "clion";
  }).env

# pkgs.mkShell {
#   name = "cpp-env";
#
#   buildInputs = with pkgs; [
#     platformio
#     jetbrains.clion
#   ];
#
#   nativeBuildInputs = with pkgs; [
#     pkg-config
#   ];
#
#
# #   shellHook = ''
# #     exec -- "${pkgs.jetbrains.clion}/bin/clion"
# #   '';
# }
