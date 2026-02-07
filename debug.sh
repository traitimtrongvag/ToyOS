#!/bin/bash

if [ ! -f build/toyos.elf ]; then
    echo "Error: build/toyos.elf not found"
    exit 1
fi

echo "Starting ToyOS in debug mode..."

qemu-system-i386 \
    -kernel build/toyos.elf \
    -serial stdio \
    -d int,cpu_reset \
    -no-reboot \
    -no-shutdown
