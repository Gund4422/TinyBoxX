; kernel_stub.s – all in one flat binary
[bits 32]
[org 0x0000]

start:
    ; Optional: setup stack
    mov ax, 0x0000
    mov ss, ax
    mov sp, 0x9FC00

    ; Call kernel code inline
    jmp kernel_start

; ---------------- Kernel code starts here ----------------
kernel_start:
    ; Example: simple infinite loop
    cli
hang:
    hlt
    jmp hang

; Fill to desired size
times 512-($-$$) db 0
