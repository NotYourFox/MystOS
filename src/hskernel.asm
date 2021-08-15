[BITS 32]

global _start

CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x00280000
    mov esp, ebp

    in al, 0x92
    or al, 2
    out 0x92, al
    
    jmp $