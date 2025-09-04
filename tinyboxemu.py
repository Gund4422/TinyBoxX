# tinyboxemu_pygame_clean.py
import pygame, sys, time, threading

# ----------------- Config -----------------
VGA_WIDTH = 80
VGA_HEIGHT = 25
CHAR_WIDTH = 10
CHAR_HEIGHT = 18
WINDOW_WIDTH = VGA_WIDTH * CHAR_WIDTH
WINDOW_HEIGHT = VGA_HEIGHT * CHAR_HEIGHT
FONT_SIZE = 16

# ----------------- Initialize Pygame -----------------
pygame.init()
screen = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT))
pygame.display.set_caption("TinyBox-OS Emulator")
font = pygame.font.SysFont("Courier", FONT_SIZE)
clock = pygame.time.Clock()

# ----------------- VGA Buffer -----------------
vga_buffer = [[" "]*VGA_WIDTH for _ in range(VGA_HEIGHT)]
vga_color = [[(255,255,255)]*VGA_WIDTH for _ in range(VGA_HEIGHT)]
term_row, term_col = 0, 0

def render_vga():
    screen.fill((0,0,0))
    for y in range(VGA_HEIGHT):
        for x in range(VGA_WIDTH):
            char = vga_buffer[y][x]
            color = vga_color[y][x]
            text = font.render(char, True, color)
            screen.blit(text, (x*CHAR_WIDTH, y*CHAR_HEIGHT))
    pygame.display.flip()

def terminal_putchar(c):
    global term_row, term_col
    if c == "\n":
        term_row += 1
        term_col = 0
    elif c == "\b":
        if term_col > 0:
            term_col -= 1
            vga_buffer[term_row][term_col] = " "
    else:
        if term_row < VGA_HEIGHT and term_col < VGA_WIDTH:
            vga_buffer[term_row][term_col] = c
            # rainbow color by row
            color = pygame.Color(0)
            color.hsva = (term_row*15 % 360, 100, 100, 100)
            vga_color[term_row][term_col] = color
            term_col += 1
    if term_row >= VGA_HEIGHT:
        vga_buffer.pop(0)
        vga_buffer.append([" "]*VGA_WIDTH)
        vga_color.pop(0)
        vga_color.append([(255,255,255)]*VGA_WIDTH)
        term_row = VGA_HEIGHT-1
    render_vga()

def terminal_write(s):
    for ch in s:
        terminal_putchar(ch)

def terminal_clear():
    global vga_buffer, vga_color, term_row, term_col
    vga_buffer = [[" "]*VGA_WIDTH for _ in range(VGA_HEIGHT)]
    vga_color = [[(255,255,255)]*VGA_WIDTH for _ in range(VGA_HEIGHT)]
    term_row, term_col = 0, 0
    render_vga()
    terminal_write("Screen cleared! Rainbow magic!\n")

# ----------------- Kernel State -----------------
shell_input = ""
uptime_ticks = 0
tasks = [{"id": i+1, "name": f"T{i+1}"} for i in range(5)]
current_task_index = 0

# ----------------- Shell -----------------
def shell_handle_char(c):
    global shell_input
    if c == "\r":
        terminal_putchar("\n")
        if shell_input == "help":
            terminal_write("Commands: help, echo, repeat, clear, uptime, date, tasks, ls, cat, echoenv\n")
        elif shell_input.startswith("echo "):
            terminal_write(shell_input[5:])
        elif shell_input.startswith("repeat "):
            parts = shell_input.split(" ", 2)
            if len(parts) == 3 and parts[1].isdigit():
                for _ in range(int(parts[1])):
                    terminal_write(parts[2] + "\n")
        elif shell_input == "clear":
            terminal_clear()
        elif shell_input == "uptime":
            terminal_write(f"Uptime ticks: {uptime_ticks}\n")
        elif shell_input == "date":
            terminal_write("Date: 2025-08-27\n")
        elif shell_input == "tasks":
            terminal_write("Tasks:\n")
            for t in tasks:
                terminal_write(f"- {t['name']}\n")
        elif shell_input == "ls":
            terminal_write("Files: file1.txt file2.txt file3.log\n")
        elif shell_input.startswith("cat "):
            terminal_write("File contents: Hello World!\n")
        elif shell_input == "echoenv":
            terminal_write("ENV=TinyBox-OS v5.2\n")
        else:
            terminal_write("Unknown command. Try help.\n")
        shell_input = ""
        terminal_write("> ")
    elif c == "\b":
        shell_input = shell_input[:-1]
        terminal_putchar("\b")
    else:
        shell_input += c
        terminal_putchar(c)

# ----------------- Background Tasks (silent) -----------------
def background_loop():
    global uptime_ticks, current_task_index
    while True:
        uptime_ticks += 1  # silent
        current_task_index = (current_task_index + 1) % len(tasks)
        time.sleep(1)  # 1 tick per second

# ----------------- Main Loop -----------------
def main_loop():
    terminal_clear()
    terminal_write("=== TinyBox-OS v5.2 Loaded ===\n> ")
    threading.Thread(target=background_loop, daemon=True).start()

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_BACKSPACE:
                    shell_handle_char("\b")
                elif event.key == pygame.K_RETURN:
                    shell_handle_char("\r")
                elif event.unicode:
                    shell_handle_char(event.unicode)
        clock.tick(60)

if __name__ == "__main__":
    main_loop()
