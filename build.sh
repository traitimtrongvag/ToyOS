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
nasm -f elf32 kernel/gdt_flush.asm -o build/gdt_flush.o
nasm -f elf32 kernel/idt_load.asm -o build/idt_load.o
nasm -f elf32 kernel/interrupt.asm -o build/interrupt.o
nasm -f elf32 kernel/asm_utils.asm -o build/asm_utils.o
nasm -f elf32 kernel/paging_asm.asm -o build/paging_asm.o

CFLAGS="-m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector -Wall -O2"

echo "[3/4] Compiling kernel..."
gcc $CFLAGS -c kernel/kernel.c        -o build/kernel.o
gcc $CFLAGS -c kernel/memory_funcs.c  -o build/memory_funcs.o
gcc $CFLAGS -c kernel/string.c        -o build/string.o
gcc $CFLAGS -c kernel/gdt.c           -o build/gdt.o
gcc $CFLAGS -c kernel/idt.c           -o build/idt.o
gcc $CFLAGS -c kernel/pic.c           -o build/pic.o
gcc $CFLAGS -c kernel/timer.c         -o build/timer.o
gcc $CFLAGS -c kernel/serial.c        -o build/serial.o
gcc $CFLAGS -c kernel/paging.c        -o build/paging.o
gcc $CFLAGS -c kernel/interrupt_handlers.c -o build/interrupt_handlers.o
gcc $CFLAGS -c kernel/irq.c           -o build/irq.o
gcc $CFLAGS -c kernel/shell.c         -o build/shell.o
gcc $CFLAGS -c kernel/task.c          -o build/task.o
gcc $CFLAGS -c kernel/heap.c          -o build/heap.o
gcc $CFLAGS -c kernel/power.c         -o build/power.o
gcc $CFLAGS -c kernel/cursor.c        -o build/cursor.o

echo "[4/4] Compiling C++ driver..."
CXXFLAGS="-m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector -fno-exceptions -fno-rtti -Wall -O2"
g++ $CXXFLAGS -c driver/driver.cpp    -o build/driver.o
g++ $CXXFLAGS -c driver/logger.cpp    -o build/logger.o
gcc $CFLAGS   -c driver/keyboard.c    -o build/keyboard.o

echo "Linking..."
GCC_LIB_PATH=$(dirname $(gcc -m32 -print-libgcc-file-name) 2>/dev/null || echo "")

OBJS="build/boot.o \
    build/gdt_flush.o build/idt_load.o build/interrupt.o \
    build/asm_utils.o build/paging_asm.o \
    build/kernel.o build/memory_funcs.o build/string.o \
    build/gdt.o build/idt.o build/pic.o build/timer.o \
    build/serial.o build/paging.o build/interrupt_handlers.o \
    build/irq.o build/shell.o build/task.o build/heap.o \
    build/power.o build/cursor.o \
    build/driver.o build/logger.o build/keyboard.o"

if [ -n "$GCC_LIB_PATH" ] && [ -f "$GCC_LIB_PATH/libgcc.a" ]; then
    ld -m elf_i386 -T kernel/linker.ld -o build/toyos.elf \
        $OBJS \
        rust_module/target/i686-unknown-linux-gnu/release/librust_module.a \
        -L"$GCC_LIB_PATH" -lgcc
else
    ld -m elf_i386 -T kernel/linker.ld -o build/toyos.elf \
        $OBJS \
        rust_module/target/i686-unknown-linux-gnu/release/librust_module.a
fi

echo "Build complete: build/toyos.elf"
echo ""
echo "Run with: qemu-system-i386 -kernel build/toyos.elf"
