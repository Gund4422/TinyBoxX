# TinyBox-OS
a OS i made in like, 1 day, it has a **605 line kernel**
# How to Compile?
its meant to be compiled using NASM and TCC!

Bootloader: nasm -f bin boot.s -o boot.bin

Kernel_Stub: nasm -f bin kernel_stub.s -o kernel_stub.bin

Kernel: tcc -m32 -ffreestanding -c kernel.c -o kernel.obj

Combine Stub and kernel: copy /b kernel_stub.bin+kernel.obj kernel.bin

Combine Boot.bin and Kernel.bin: copy /b boot.bin+kernel.bin tinyboxos.img
run emu: python3 tinyboxemu.py
# Why is the OS called "TinyBox?"
Well.. its because its a Tiny OS running in my emu, it has fully lost its motto from running in bochs ): 
# What Does It use?
- **93% C** → kernel, shell, task scheduler, VGA driver, timer, syscalls  
- **6% Assembly** → bootloader, kernel stub, low-level setup
# My Note  
Intiha's Note: Intiha's Note: THIS OS WAS MADE FROM SCRATCH.
It serves as a template for people using TCC and NASM.
