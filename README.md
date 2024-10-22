# PYNQ UARTKit

This is a library to communicate asynchronously between multiple clients using 
UART.

This program is written using TU Eindhoven’s 
[libpynq](https://pynq.tue.nl/5ewc0/libpynq/) library.

> **NOTE:** This is a work in progress, and might not work.

## Instructions

The Makefile for this program has been written in such a way as to cross compile
from a separate machine and transfer to the PYNQ-board.

You might want to configure the build before running `make`. 

In broad terms, steps to build are as follows:

1. Make sure you have a compiled version of `libpynq` from TU/e.
   + The compiled library is expected at __../../libpynq/lib/libpynq.a__
   + Headers are expected at __../../libpynq/include/…__
   + Change the `LIBRARIES` variable in `Makefile` if necessary.

2. Make sure you have a sysroot from the PYNQ board.
   + You can ssh into the PYNQ-board and archive `/lib /usr/include /usr/lib /usr/local/lib /usr/local/include`
   + The sysroot is expected at __../../sysroot__
   + Change the `SYSROOT` variable in `Makefile` if necessary.

3. Make sure you have a compiler and linker that supports the PYNQ-board
   + The target triple for PYNQ-Z2 is `arm-linux-gnueabihf`
   + I had success with Apple `clang` bundled with Xcode and and `lld` installed with Homebrew. 
   + In Makefile, clang is told to use lld for linking with `fuse-ld=lld`

4. Compile
   + Run `make` in order to compile.
   + Object files are put into __build/artifacts__
   + The output library is put into __build/bin__
   + Headers are put into __build/include__

## Roadmap
- [ ] Possible linker issues
- [ ] Using a map to store listener registrations
- [ ] Testing communication between two boards
- [ ] Testing communication between multiple boards
- [ ] Locking and unlocking channel so messages are not mangled

## Editing source code in VS Code

For code completion, error checking and navigation I recommend using the `clangd` extension in VS Code.

You can add a __.clangd__ file in the project root specifying where to find the libpynq headers like so:

```yaml
CompileFlags:
  Add:
    - "-I/Users/.../Documents/Projects/PYNQ/libpynq/include"
    - "--sysroot=/Users/.../Documents/Projects/PYNQ/sysroot"
    - "--target=arm-linux-gnueabihf"
```