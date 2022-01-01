section .asm
global tss_load
extern log

tss_load:
    push ebp
    mov ebp, esp
    mov ax, [ebp+8]
    ltr ax
    pop ebp
    ret