; Minimal Multiboot2 compliant bootloader
; OSDev Wiki reference implementation

; Multiboot2 Header
section .multiboot_header
align 8
mb_header:
    dd 0xe85250d6                ; Magic number
    dd 0                         ; Architecture (i386)
    dd mb_header_end - mb_header ; Header length
    ; Checksum: -(magic + architecture + header_length)
    dd 0x100000000 - (0xe85250d6 + 0 + (mb_header_end - mb_header))
    
    ; Required end tag
    dw 0    ; type = 0
    dw 0    ; flags = 0
    dd 8    ; size = 8
mb_header_end:

section .text
global start
extern kernel_main

start:
    cli
    mov esp, stack_top
    push ebx    ; Multiboot info pointer
    push eax    ; Multiboot magic
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
