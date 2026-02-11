BITS 32

MULTIBOOT_HEADER_MAGIC equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS equ 0x00000003
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

KERNEL_STACK_SIZE equ 0x4000

section .multiboot
align 4
    dd MULTIBOOT_HEADER_MAGIC
    dd MULTIBOOT_HEADER_FLAGS
    dd MULTIBOOT_CHECKSUM

section .bss
align 16
stack_bottom:
    resb KERNEL_STACK_SIZE
stack_top:

section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top
    
    push eax
    push ebx
    
    call kernel_main
    
    cli
.hang:
    hlt
    jmp .hang

section .note.GNU-stack noalloc noexec nowrite progbits
