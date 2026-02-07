.PHONY: all clean run

NASM = nasm
CC ?= gcc
CXX ?= g++
LD ?= ld

CFLAGS = -m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector -Wall -O2
CXXFLAGS = -m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector -fno-exceptions -fno-rtti -Wall -O2
LDFLAGS = -m elf_i386 -T kernel/linker.ld

ifeq ($(shell uname -o 2>/dev/null),Android)
    CC = clang
    CXX = clang++
    LD = ld.lld
    QEMU = qemu-system-x86_64 -cpu qemu32
    LDFLAGS += --no-dynamic-linker
else
    QEMU = qemu-system-i386
    LDFLAGS += -L/usr/lib/gcc/i686-linux-gnu/13 -lgcc
endif

BOOT_OBJ = build/boot.o
KERNEL_OBJ = build/kernel.o
DRIVER_OBJ = build/driver.o
RUST_LIB = rust_module/target/i686-unknown-linux-gnu/release/librust_module.a

all: build/toyos.elf

build/boot.o: boot/boot.asm
	mkdir -p build
	$(NASM) -f elf32 boot/boot.asm -o build/boot.o

build/kernel.o: kernel/kernel.c
	mkdir -p build
	$(CC) $(CFLAGS) -c kernel/kernel.c -o build/kernel.o

build/driver.o: driver/driver.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c driver/driver.cpp -o build/driver.o

$(RUST_LIB):
	cd rust_module && cargo build --release --target i686-unknown-linux-gnu

build/toyos.elf: $(BOOT_OBJ) $(KERNEL_OBJ) $(DRIVER_OBJ) $(RUST_LIB)
	$(LD) $(LDFLAGS) -o build/toyos.elf $(BOOT_OBJ) $(KERNEL_OBJ) $(DRIVER_OBJ) $(RUST_LIB)

run: build/toyos.elf
	$(QEMU) -kernel build/toyos.elf -nographic -serial mon:stdio

clean:
	rm -rf build
	cd rust_module && cargo clean
