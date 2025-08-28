# TinyBox-OS
a OS i made in like, 1 day, it has a **724 line kernel**
# How to Compile?
its meant to be compiled using NASM and TCC!

Bootloader: nasm -f bin boot.s -o boot.bin

Kernel_Stub: nasm -f bin kernel_stub.s -o kernel_stub.bin

Kernel: tcc -m32 -ffreestanding -nostdlib -c kernel.c -o kernel.obj

Combine Stub and kernel: copy /b kernel_stub.bin+kernel.obj kernel.bin

Combine Boot.bin and Kernel.bin: copy /b boot.bin+kernel.bin tinyboxos.img

Run with: bochs -f bochsrc.txt

Bochsrc.txt: floppya: 1_44=tinyboxos.img, status=inserted, boot: a, megs: 16, romimage: file=BIOS-bochs-latest, vgaromimage: file=VGABIOS-lgpl-latest, log: bochslog.txt
# Why is the OS called "TinyBox?"
Well.. its because its a Tiny OS running in Bochs (Box)! Creative, Right?
# What Does It use?
- **93% C** → kernel, shell, task scheduler, VGA driver, timer, syscalls  
- **6% Assembly** → bootloader, kernel stub, low-level setup
# My Note  
Intiha's Note: Intiha's Note: THIS OS WAS MADE FROM SCRATCH.
It serves as a template for people using TCC and NASM.
