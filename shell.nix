{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  buildInputs = with pkgs;
    [
      vscode
      libusb1                     # Needed by picotool
      hidapi                      # Needed by picotool
      ninja
      cmake
      python3
      udisks                      # Interact with bootloader filesystem
      tio                         # terminal program to interface with serial
      llvmPackages_19.clang-tools # clang-format, clang-tidy
      git
    ];
    # We don't install the pico sdk here; that should be handled by the Pico VS Code
    # extension
  shellHook = ''
    export LD_LIBRARY_PATH=${pkgs.lib.makeLibraryPath [ pkgs.libusb1 pkgs.hidapi pkgs.stdenv.cc.cc.lib ]}''${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
  '';
}
