.PHONY: all clean run debug help

# Tool definitions
NASM = nasm
CC ?= gcc
CXX ?= g++
LD ?= ld

# Compiler flags
CFLAGS = -m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector -Wall -Wextra -O2
CXXFLAGS = -m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector -fno-exceptions -fno-rtti -Wall -Wextra -O2
LDFLAGS = -m elf_i386 -T kernel/linker.ld

# Platform-specific settings
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

# Source and object files
BOOT_SRC = boot/boot.asm
KERNEL_SRC = kernel/kernel.c
DRIVER_SRC = driver/driver.cpp

BOOT_OBJ = build/boot.o
KERNEL_OBJ = build/kernel.o
DRIVER_OBJ = build/driver.o
RUST_LIB = rust_module/target/i686-unknown-linux-gnu/release/librust_module.a

# Header dependencies
KERNEL_HEADERS = $(wildcard kernel/*.h)
DRIVER_HEADERS = $(wildcard driver/*.h)

# Default target
all: build/toyos.elf

# Help target
help:
	@echo "ToyOS Build System"
	@echo "=================="
	@echo "Targets:"
	@echo "  all     - Build the kernel (default)"
	@echo "  run     - Build and run in QEMU"
	@echo "  debug   - Build and run in QEMU with debug options"
	@echo "  clean   - Remove all build artifacts"
	@echo "  help    - Show this help message"

# Boot object
build/boot.o: $(BOOT_SRC)
	@mkdir -p build
	@echo "Assembling bootloader..."
	$(NASM) -f elf32 $(BOOT_SRC) -o $(BOOT_OBJ)

# Kernel object with header dependencies
build/kernel.o: $(KERNEL_SRC) $(KERNEL_HEADERS)
	@mkdir -p build
	@echo "Compiling kernel..."
	$(CC) $(CFLAGS) -c $(KERNEL_SRC) -o $(KERNEL_OBJ)

# Driver object with header dependencies
build/driver.o: $(DRIVER_SRC) $(DRIVER_HEADERS)
	@mkdir -p build
	@echo "Compiling C++ driver..."
	$(CXX) $(CXXFLAGS) -c $(DRIVER_SRC) -o $(DRIVER_OBJ)

# Rust library
$(RUST_LIB): rust_module/src/*.rs rust_module/Cargo.toml
	@echo "Building Rust module..."
	cd rust_module && cargo build --release --target i686-unknown-linux-gnu

# Final kernel binary
build/toyos.elf: $(BOOT_OBJ) $(KERNEL_OBJ) $(DRIVER_OBJ) $(RUST_LIB)
	@echo "Linking kernel..."
	$(LD) $(LDFLAGS) -o build/toyos.elf $(BOOT_OBJ) $(KERNEL_OBJ) $(DRIVER_OBJ) $(RUST_LIB)
	@echo "Build complete: build/toyos.elf"

# Run in QEMU
run: build/toyos.elf
	@echo "Starting QEMU..."
	$(QEMU) -kernel build/toyos.elf -nographic -serial mon:stdio

# Debug in QEMU with gdb support
debug: build/toyos.elf
	@echo "Starting QEMU with GDB support..."
	$(QEMU) -kernel build/toyos.elf -nographic -serial mon:stdio -s -S

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf build
	cd rust_module && cargo clean
	@echo "Clean complete"
