cmake_minimum_required(VERSION 3.16)
project(ToyOS ASM C CXX)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_ASM_NASM_COMPILER nasm)

set(CMAKE_C_FLAGS "-m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector -Wall")
set(CMAKE_CXX_FLAGS "-m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector -fno-exceptions -fno-rtti -Wall")

enable_language(ASM_NASM)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)
set(CMAKE_ASM_NASM_FLAGS "-f elf32")

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/rust_module/librust_module.a
    COMMAND cargo build --release --target i686-toyos.json
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/rust_module
    COMMENT "Building Rust module"
)

add_custom_target(rust_lib ALL
    DEPENDS ${CMAKE_BINARY_DIR}/rust_module/librust_module.a
)

add_executable(toyos.elf
    boot/boot.asm
    kernel/kernel.c kernel/string.c kernel/gdt.c kernel/idt.c kernel/pic.c
    kernel/timer.c kernel/paging.c kernel/interrupt_handlers.c kernel/irq.c
    kernel/shell.c kernel/task.c kernel/heap.c kernel/power.c kernel/cursor.c
    kernel/memory_funcs.c kernel/syscall.c kernel/vfs_test.c kernel/heap_test.c
    kernel/paging_test.c kernel/task_test.c kernel/syscall_test.c
    driver/driver.cpp driver/keyboard.c driver/logger.cpp driver/rtc.c
    driver/speaker.c
)

target_link_libraries(toyos.elf
    ${CMAKE_SOURCE_DIR}/rust_module/target/i686-toyos.json/release/librust_module.a
    gcc
)

set_target_properties(toyos.elf PROPERTIES
    LINK_FLAGS "-m32 -T ${CMAKE_SOURCE_DIR}/kernel/linker.ld -nostdlib -lgcc"
    LINK_DEPENDS ${CMAKE_SOURCE_DIR}/kernel/linker.ld
)

add_dependencies(toyos.elf rust_lib)

add_custom_target(run
    COMMAND qemu-system-i386 -kernel toyos.elf
    DEPENDS toyos.elf
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running ToyOS in QEMU"
)
