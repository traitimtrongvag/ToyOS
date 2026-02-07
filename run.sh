#!/bin/bash

if [ ! -f build/toyos.elf ]; then
    echo "Error: build/toyos.elf not found"
    echo "Run ./build.sh first"
    exit 1
fi

echo "Starting ToyOS in QEMU..."
echo "Press Ctrl+A then X to exit QEMU"
echo ""

qemu-system-i386 -kernel build/toyos.elf -serial stdio
