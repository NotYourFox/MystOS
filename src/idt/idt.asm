section .asm

extern INT21H_HANDLER
extern no_int_handler

global idt_load
global INT21H
global no_int
global interrupt_flag

interrupt_flag:
    push ebp
    mov ebp, esp
    mov ebx, [ebp+8]
    cmp ebx, 0
    je .clrint
    jmp .setint
    .clrint:
        cli
        pop ebp
        ret
    .setint:
        sti
        pop ebp
        ret

idt_load:
    push ebp
    mov ebp, esp
    mov ebx, [ebp+8]
    lidt [ebx]
    pop ebp
	ret



INT21H:
    cli
    pushad
    call INT21H_HANDLER
    popad
    sti
    iret

no_int:
    cli
    pushad
    call no_int_handler
    popad
    sti
    iret

