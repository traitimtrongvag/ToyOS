BITS 32

section .text

global enable_interrupts
global disable_interrupts
global halt_cpu
global read_cr0
global write_cr0
global read_cr3
global write_cr3

enable_interrupts:
    sti
    ret

disable_interrupts:
    cli
    ret

halt_cpu:
    hlt
    ret

read_cr0:
    mov eax, cr0
    ret

write_cr0:
    mov eax, [esp + 4]
    mov cr0, eax
    ret

read_cr3:
    mov eax, cr3
    ret

write_cr3:
    mov eax, [esp + 4]
    mov cr3, eax
    ret
