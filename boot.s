[org 0x7C00]

start:
    cli                     ; Disable interrupts
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00          ; Temporary stack

    mov bx, 0x8000          ; Load address for kernel
    mov es, bx
    mov ah, 0x02            ; BIOS read sector
    mov al, 32              ; Kernel size in sectors (16 KB)
    mov ch, 0               ; Cylinder
    mov cl, 2               ; Sector start
    mov dh, 0               ; Head
    mov dl, 0x00            ; Boot drive
    int 0x13                ; Read kernel

    jmp 0x8000:0000         ; Jump to kernel

; Fill up to 510 bytes
times 510-($-$$) db 0
dw 0xAA55
