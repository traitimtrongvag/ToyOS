#!/bin/bash

set -e

echo "Building ToyOS..."

mkdir -p build

echo "[1/4] Building Rust module..."
cd rust_module
cargo build --release --target i686-unknown-linux-gnu
cd ..

echo "[2/4] Assembling bootloader..."
nasm -f elf32 boot/boot.asm -o build/boot.o

echo "[3/4] Compiling kernel..."
gcc -m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector -Wall -O2 \
    -c kernel/kernel.c -o build/kernel.o

echo "[4/4] Compiling C++ driver..."
g++ -m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector \
    -fno-exceptions -fno-rtti -Wall -O2 \
    -c driver/driver.cpp -o build/driver.o

echo "Linking..."
ld -m elf_i386 -T kernel/linker.ld -o build/toyos.elf \
    build/boot.o build/kernel.o build/driver.o \
    rust_module/target/i686-unknown-linux-gnu/release/librust_module.a \
    -L/usr/lib/gcc/i686-linux-gnu/13 -lgcc

echo "Build complete: build/toyos.elf"
echo ""
echo "Run with: qemu-system-i386 -kernel build/toyos.elf"
