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
KERNEL_SRC = kernel/kernel.c kernel/string.c kernel/gdt.c kernel/pic.c kernel/serial.c kernel/timer.c kernel/idt.c kernel/paging.c
DRIVER_SRC = driver/driver.cpp driver/keyboard.c driver/logger.cpp
ASM_SRC = kernel/asm_utils.asm kernel/gdt_flush.asm kernel/interrupt.asm kernel/idt_load.asm kernel/paging_asm.asm

BOOT_OBJ = build/boot.o
KERNEL_OBJ = $(patsubst kernel/%.c,build/%.o,$(filter %.c,$(KERNEL_SRC)))
ASM_OBJ = $(patsubst kernel/%.asm,build/%.o,$(ASM_SRC))
DRIVER_OBJ = $(patsubst driver/%.cpp,build/driver_%.o,$(filter %.cpp,$(DRIVER_SRC))) \
             $(patsubst driver/%.c,build/driver_%.o,$(filter %.c,$(DRIVER_SRC)))
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

# Kernel C objects
build/%.o: kernel/%.c $(KERNEL_HEADERS)
	@mkdir -p build
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Assembly objects
build/%.o: kernel/%.asm
	@mkdir -p build
	@echo "Assembling $<..."
	$(NASM) -f elf32 $< -o $@

# C++ driver objects
build/driver_%.o: driver/%.cpp $(DRIVER_HEADERS)
	@mkdir -p build
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# C driver objects
build/driver_%.o: driver/%.c $(DRIVER_HEADERS)
	@mkdir -p build
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Rust library
$(RUST_LIB): rust_module/src/*.rs rust_module/Cargo.toml
	@echo "Building Rust module..."
	cd rust_module && cargo build --release --target i686-unknown-linux-gnu

# Final kernel binary
build/toyos.elf: $(BOOT_OBJ) $(KERNEL_OBJ) $(ASM_OBJ) $(DRIVER_OBJ) $(RUST_LIB)
	@echo "Linking kernel..."
	$(LD) $(LDFLAGS) -o build/toyos.elf $(BOOT_OBJ) $(KERNEL_OBJ) $(ASM_OBJ) $(DRIVER_OBJ) $(RUST_LIB)
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
