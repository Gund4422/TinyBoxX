# tinyboxemu.py
import sys

# ------------------ Configuration ------------------
MEM_SIZE = 1024 * 1024  # 1 MB RAM
BOOT_SECTOR = 0x7C00
KERNEL_LOAD = 0x80000  # where the kernel gets loaded
VGA_START = 0xB8000
VGA_WIDTH = 80
VGA_HEIGHT = 25

# ------------------ Memory ------------------
memory = bytearray(MEM_SIZE)

# Load floppy image
if len(sys.argv) < 2:
    print("Usage: python tinyboxemu.py tinyboxos.img")
    sys.exit(1)

with open(sys.argv[1], "rb") as f:
    floppy = f.read()
    memory[0:len(floppy)] = floppy

# ------------------ VGA Text Mode ------------------
vga_buffer = [" "] * (VGA_WIDTH * VGA_HEIGHT)

def vga_write(offset, char):
    if 0 <= offset < len(vga_buffer):
        vga_buffer[offset] = chr(char)
        render_vga()

def render_vga():
    # Simple terminal render
    print("\033[H")  # move cursor to top-left
    for y in range(VGA_HEIGHT):
        line = "".join(vga_buffer[y*VGA_WIDTH:(y+1)*VGA_WIDTH])
        print(line)
    print("-" * VGA_WIDTH)

# ------------------ CPU Registers ------------------
regs = {
    "EIP": BOOT_SECTOR,
    "AX": 0,
    "BX": 0,
    "CX": 0,
    "DX": 0,
    "DS": 0,
    "ES": 0,
    "SS": 0,
    "SP": 0x7C00,
    "FLAGS": 0,
}

# ------------------ BIOS Interrupts ------------------
def int_0x10():
    # Only simple text output
    ah = (regs["AX"] >> 8) & 0xFF
    al = regs["AX"] & 0xFF
    if ah == 0x0E:
        # Teletype output
        print(chr(al), end="", flush=True)

def int_0x13():
    # Only read sectors from floppy (simple)
    drive = regs["DX"] & 0xFF
    head = (regs["DX"] >> 8) & 0xFF
    sector = regs["CX"] & 0xFF
    cyl = (regs["CX"] >> 8) & 0xFF
    num = regs["AX"] & 0xFF
    addr = regs["BX"]
    # naive: just copy from floppy image
    start = (sector-1 + cyl*18 + head*18*2) * 512
    memory[addr:addr+num*512] = floppy[start:start+num*512]

# ------------------ Minimal CPU loop ------------------
def run():
    eip = regs["EIP"]
    steps = 0
    while True:
        opcode = memory[eip]
        # For now, we only support CLI (0xFA) and HLT (0xF4)
        if opcode == 0xFA:  # CLI
            # Disable interrupts (no-op)
            eip += 1
        elif opcode == 0xF4:  # HLT
            print("\nCPU halted.")
            break
        else:
            print(f"Unsupported opcode {opcode:02X} at {eip:05X}")
            break
        steps += 1
        if steps > 1000:
            print("Stopping after 1000 steps to avoid infinite loop")
            break

if __name__ == "__main__":
    run()
