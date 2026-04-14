{
  description = "Pico development shell";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
         pkgs = import nixpkgs {
          inherit system;
          config.allowUnfree = true;
        };
        gdb-multiarch = pkgs.runCommand "gdb-multiarch" {} ''
          mkdir -p $out/bin 
          ln -s ${pkgs.gdb}/bin/gdb $out/bin/gdb-multiarch
        '';
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            vscode
            libusb1                     # Needed by picotool
            hidapi                      # Needed by picotool
            ninja
            cmake
            python3
            udisks                      # Interact with bootloader filesystem
            tio                         # terminal program to interface with serial
            llvmPackages_19.clang-tools # clang-format, clang-tidy
            libftdi1                    # Pico vscode extension
            git
            gdb
            gdb-multiarch
          ];
          # We don't install the pico sdk here; that should be handled by the
          # Pico VS Code extension
          shellHook = ''
            export LD_LIBRARY_PATH=${pkgs.lib.makeLibraryPath [ pkgs.libusb1 pkgs.hidapi pkgs.libftdi1 pkgs.stdenv.cc.cc.lib ]}''${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
          '';
        };
      });
}
