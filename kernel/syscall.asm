bits 32

extern syscall_dispatch

global syscall_handler
global syscall_stub

; System call handler entry point
syscall_handler:
    push ebp
    mov ebp, esp
    
    ; Save all registers
    pushad
    push ds
    push es
    push fs
    push gs
    
    ; Load kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; System call arguments from registers
    ; EAX = syscall number
    ; EBX = arg1
    ; ECX = arg2
    ; EDX = arg3
    ; ESI = arg4
    ; EDI = arg5
    
    push edi
    push esi
    push edx
    push ecx
    push ebx
    push eax
    
    ; Call C dispatcher
    call syscall_dispatch
    
    ; Clean up arguments
    add esp, 24
    
    ; Save return value
    mov [esp + 28], eax
    
    ; Restore registers
    pop gs
    pop fs
    pop es
    pop ds
    popad
    
    pop ebp
    iret

; Stub for triggering syscall from kernel mode
syscall_stub:
    int 0x80
    ret
