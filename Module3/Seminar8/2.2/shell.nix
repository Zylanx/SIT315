{pkgs ? import <nixpkgs> {}}:
let
    opencl_vector_ops = pkgs.callPackage ./opencl_vector_ops.nix {};
in
pkgs.mkShell {
    shellHook = ''
        ${opencl_vector_ops}/bin/vector_ops
        exit
    '';
}