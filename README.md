# windOS — Minimalist x86 Bare-Metal Kernel

A bare-metal 32-bit x86 micro-kernel operating in VGA text mode, featuring a shell, a DOS-like interpreter, and a tiny BASIC interpreter in ~15 KB with zero dependencies.

This project is part of AI Code Laboratory, a collection of projects developed with the assistance or generation of artificial intelligence.

## Description

windOS is a self-contained OS kernel designed to boot directly on x86 hardware or emulators via Multiboot specification. It runs completely freestanding without any underlying operating system, C standard library (libc), or third-party dependencies.

The kernel includes:
- Multiboot compliance (compatible with GRUB or direct QEMU kernel booting)
- VGA text mode display driver (80x25 characters, custom colors, scrolling)
- PS/2 keyboard driver using hardware polling
- Main system shell with built-in utility commands (`help`, `ver`, `cls`, `color`, `echo`, `reboot`, `halt`)
- Simulated DOS-like command interpreter (`dos`) with an in-memory virtual filesystem
- Minimal BASIC interpreter (`basic`) capable of parsing and executing integer-based programs (`PRINT`, `LET`, `IF/THEN`, `GOTO`, `FOR/NEXT`, `INPUT`)

## Technologies

- C (Freestanding GNU C99)
- x86 Assembly (GNU Assembler syntax)
- Multiboot Specification
- Custom Linker Script (`linker.ld`)
- Claude (Anthropic), used to generate the code

## Installation

Building the kernel requires standard x86 cross-compilation tools (`gcc`, `as`, `ld`, `make`). On Debian/Ubuntu systems, install `build-essential`.

Clone the repository:
```bash
git clone [https://github.com/AI-Code-Laboratory/windos.git](https://github.com/AI-Code-Laboratory/windos.git)
