# ToyOS

A minimal operating system kernel demonstrating multi-language systems programming.

## About

ToyOS is an educational OS kernel that combines Assembly, C, Rust, and C++ to illustrate fundamental operating system concepts. The project emphasizes understanding low-level programming, cross-language FFI, and bare-metal development.

## Architecture

The kernel consists of four language layers working together:

- Assembly: Multiboot bootloader and hardware initialization
- C: Core kernel functionality and VGA terminal driver  
- Rust: Memory management with page frame allocator
- C++: Device driver layer with OOP abstractions

## Building

Prerequisites:
- NASM assembler
- GCC/G++ with 32-bit multilib support
- Rust toolchain (install i686-unknown-linux-gnu target)
- GNU ld linker
- QEMU for testing

Install Rust target:
```bash
rustup target add i686-unknown-linux-gnu
```

Build the kernel:
```bash
./build.sh
```

Run in QEMU:
```bash
./run.sh
```

Or use Make:
```bash
make
make run
```

## Project Structure

```
boot/           Multiboot bootloader (Assembly)
kernel/         Core kernel (C)
driver/         Device drivers (C++)
rust_module/    Memory manager (Rust)
docs/           Technical documentation
```

## Documentation

See the `docs/` directory for detailed technical information:

- Boot process and initialization
- Memory layout and allocation
- Build system and linking
- API reference
- ABI compatibility

## Learning Objectives

This project demonstrates:

- x86 protected mode programming
- Cross-language linking and FFI
- Freestanding code without standard libraries
- Memory management fundamentals
- VGA text mode driver implementation

## License

Educational use. Modify freely for learning purposes.

## Resources

- OSDev Wiki: https://wiki.osdev.org
- Intel x86 manuals
- System V i386 ABI specification