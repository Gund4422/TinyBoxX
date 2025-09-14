# TinyBoxX

A modified version of TinyBox OS, now even tinier — 100+ lines of kernel magic, woohoo! 🎉

TinyBoxX is a small, experimental OS that runs in my custom emulator. It keeps all the core features of TinyBox OS, including VGA output, shell, task scheduler, timer, and syscalls — but now optimized and simplified for maximum compactness.

# How to Compile

TinyBoxX is designed to be compiled using NASM and TCC.

Build Steps (Windows)
:: Compile bootloader
nasm -f bin boot.s -o boot.bin

:: Compile kernel stub
nasm -f bin kernel_stub.s -o kernel_stub.bin

:: Compile kernel
tcc -m32 -ffreestanding -c kernel.c -o kernel.obj

:: Combine stub and kernel
copy /b kernel_stub.bin+kernel.obj kernel.bin

:: Combine bootloader and kernel into final image
copy /b boot.bin+kernel.bin tinyboxos.img

Build Steps (Linux/macOS)
nasm -f bin boot.s -o boot.bin
nasm -f bin kernel_stub.s -o kernel_stub.bin
tcc -m32 -ffreestanding -c kernel.c -o kernel.obj
cat boot.bin kernel_stub.bin kernel.obj > tinyboxos.img

# How to Run

Run TinyBoxX using the custom Python emulator:

python3 tinyboxemu.py


Or use QEMU for quick testing:

qemu-system-i386 -fda tinyboxos.img

# Why is it called "TinyBox"?

Because it’s a tiny OS running in my emulator!
It has fully lost its original motto from running in Bochs, but hey — it’s still tiny, still fun. 😎

What Does It Use?

93% C → Kernel, shell, task scheduler, VGA driver, timer, syscalls

6% Assembly → Bootloader, kernel stub, low-level setup

The kernel itself is ~450 LOC after optimization, with all TinyBoxX features intact.

Features

VGA Terminal: Colors, scrolling, and character output

Shell: help, echo, repeat, clear, uptime, date, tasks, ls, cat, echoenv

Task Scheduler: Round-robin multitasking with dummy tasks

Timer: Tick counter and debug prints

Syscalls: Stubbed for future expansion

Keyboard Input: Character handling with backspace support
