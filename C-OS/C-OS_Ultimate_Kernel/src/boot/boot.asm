section .multiboot_header
align 8
header_start:
    dd 0xe85250d6                ; Magic
    dd 0                         ; Architecture (i386)
    dd header_end - header_start ; Header length
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ; Checksum
    
    ; End tag (type 0, size 8)
    dw 0
    dw 0
    dd 8
header_end:
 
section .text
global start
global irq12_handler
extern kernel_main
extern mouse_handler

; IRQ12 handler for PS/2 mouse
; IRQ12 = slave PIC IRQ4 = interrupt 0x2C
irq12_handler:
    pusha
    push ds
    push es
    push fs
    push gs
    
    ; Call C++ mouse handler
    call mouse_handler
    
    ; Send EOI to slave PIC
    mov al, 0x20
    out 0xA0, al
    
    ; Send EOI to master PIC
    out 0x20, al
    
    pop gs
    pop fs
    pop es
    pop ds
    popa
    iret

start:
    cli
    mov esp, stack_top
    push ebx
    push eax
    call kernel_main
    cli
.halt:
    hlt
    jmp .halt
 
section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
