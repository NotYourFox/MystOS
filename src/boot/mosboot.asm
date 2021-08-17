ORG 7C00h
[BITS 16]

CODE_SEG equ GDT_CODE - GDT_START
DATA_SEG equ GDT_DATA - GDT_START

_start:
    jmp short start
    nop

times 33 db 0

start:
    jmp 0x0:load

load:
    cli
    mov ax, 0x00
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x00
    sti

    call find_version_file

    mov ah, 2
    mov al, 1
    mov ch, 0
    mov dh, 0
    mov bx, buffer
    int 13h
    jc read_err

    mov cx, bootmsg
    call print
    call find_version_text
    mov cx, bx
    call print
    mov cx, dots
    call print
    mov cx, newline
    call print
    mov ax, 0xFFFF
    mov cx, 0x2000
    call sleep

.load_protected:
    cli
    lgdt[GDT_DESCRIPTOR]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:load32

find_version_file:
    push ax
    push bx
    push dx
    mov cl, 1
    .read_next:
        mov ah, 2
        mov al, 1
        mov ch, 0
        mov dh, 0
        mov bx, buffer
        int 13h
        jc .next_cluster
        mov ax, bx
        .next_byte:
            cmp byte[bx], 0x76
            je .cmp_next
            .continue_finding:
                push bx
                sub bx, ax
                cmp bx, 510
                je .popbx
                pop bx
                inc bx
                jmp .next_byte
        .cmp_next:
            inc bx
            cmp byte[bx], 0x2E
            je .finish_fv
            dec bx
            jmp .continue_finding
        .next_cluster:
            inc cl
            jmp .read_next
        .popbx:
            pop bx
            jmp .next_cluster
    .finish_fv:
        pop dx
        pop bx
        pop ax
        ret
            
find_version_text:
    .next_byte:
        cmp byte[bx], 0x76
        je .cmp_next
        .continue_finding:
            inc bx
            jmp .next_byte
    .cmp_next:
        inc bx
        cmp byte[bx], 0x2E
        je .finish_fvt
        dec bx
        jmp .continue_finding
    .finish_fvt:
        dec bx
        call truncate_linefeed
        ret

truncate_linefeed:
    push bx
    .next_byte:
        cmp byte[bx], 0x0A
        je .truncate
        .continue_finding:
            inc bx
            jmp .next_byte
    .truncate:
        mov byte[bx], 0
        pop bx
        ret


print:
    push si
    push bx
    push ax
    mov si, cx
    mov bx, 0
    print_loop:
        lodsb
        cmp al, 0
        jz finish_print
        call PrintChar
        jmp print_loop
    finish_print:
        pop ax
        pop bx
        pop si
        ret

PrintChar:
    mov ah, 0Eh
    int 10h
    ret

sleep:
    push ax
    push cx
    sleep_loop:
        cmp cx, 0
        jz dec_ax
        dec cx
        jmp sleep_loop
    dec_ax:
        cmp ax, 0
        jz finish_sleep
        dec ax
        pop cx
        push cx
        jmp sleep_loop
    finish_sleep:
        pop cx
        pop ax
        ret

read_err:
    mov cx, read_err_msg
    call print
    cli
    hlt
    ret

GDT_START:

GDT_NULL: ;64 bits of zeros
    dd 0x0 
    dd 0x0

; offset 0x8
GDT_CODE:     ;CS SHOULD POINT TO THIS
    dw 0xFFFF ;Segment limit first 0-15 bits
    dw 0      ;Base first 0-15 bits
    db 0      ;Base 16-23 bits
    db 0x9a   ;Access byte
    db 11001111b ;High and low 4 bit flags
    db 0      ;Base 24-31 bits

; offset 0x10
GDT_DATA:     ;DS, SS, ES, FS, GS
    dw 0xFFFF ;Segment limit first 0-15 bits
    dw 0      ;Base first 0-15 bits
    db 0      ;Base 16-23 bits
    db 0x92   ;Access byte
    db 11001111b ;High and low 4 bit flags
    db 0      ;Base 24-31 bits

GDT_END:

GDT_DESCRIPTOR:
    dw GDT_END - GDT_START - 1
    dd GDT_START

[BITS 32]

load32:
    mov eax, 1
    mov ecx, 128
    mov edi, 0x0100000
    call ata_lba_read
    jmp CODE_SEG:0x0100000

ata_lba_read:
    mov ebx, eax ;Backup the LBA
    ;Send the highest 8 bits of the LBA to harddisk controller
    shr eax, 24 ;Shift 24 bits so eax now contain only the highest 8 bits of the LBA
    or eax, 0xE0 ;Select the master drive
    mov dx, 0x1F6
    out dx, al ;Send
    ;Send the total sectors to read
    mov eax, ecx
    mov dx, 0x1F2
    out dx, al
    ;Send more bits of the LBA
    mov eax, ebx ;Restore LBA
    mov dx, 0x1F3
    out dx, al
    ;Send more bits of the LBA
    mov eax, ebx ;Restore LBA
    mov dx, 0x1F4
    shr eax, 8 ;Shift 8 bits that were sent
    out dx, al
    ;Send upper 16 bits of the LBA
    mov eax, ebx ;Restore LBA
    mov dx, 0x1F5
    shr eax, 16 ;Shift 16 bits
    out dx, al
    ;Finished sending
    mov dx, 0x1F7
    mov al, 0x20
    out dx, al
    ;Read all sectors
    .next_sector:
        push ecx
    ;Checking the need to read
    .try_again:
        mov dx, 0x1F7
        in al, dx
        test al, 8
        jz .try_again
    ;We need to read 256 words at a time
        mov ecx, 256
        mov dx, 0x1F0
        rep insw
        pop ecx
        loop .next_sector
    ;End of reading
        ret


bootmsg db 'MOSBoot now loading MystOS ', 0
dots db '...', 0
newline db 0Ah, 0Dh, 0
read_err_msg db 0Ah, 0Dh, 'Cluster read error!', 0
times 510-($-$$) db 0
dw 0xAA55

buffer: