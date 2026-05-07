; Simple Multiboot2 compliant bootloader
; Based on OSDev wiki example

; Multiboot2 Header - Must be 64-bit aligned
section .multiboot_header
align 8
    ; Multiboot2 header magic
    dd 0xe85250d6
    ; Architecture (0 = i386)
    dd 0
    ; Header length
    dd header_end - header_start
header_start:
    ; Checksum
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))
    
    ; End tag (required)
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
header_end:

; Code section
section .text
global start
extern kernel_main

start:
    ; Disable interrupts
    cli
    
    ; Setup stack
    mov esp, stack_top
    
    ; Clear BSS (optional but good practice)
    ; Call kernel
    call kernel_main
    
    ; Halt if kernel returns
    cli
.halt:
    hlt
    jmp .halt

; BSS section
section .bss
align 16
stack_bottom:
    resb 16384  ; 16KB stack
stack_top:
