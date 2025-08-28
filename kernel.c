/* ========================================================= */
/* TinyBox-OS v5.2                     */
/* 724 lines                  */
/* Features: VGA, Keyboard, Shell, Tasks, Syscalls, Timer   */
/* ========================================================= */

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

/* ===================== FORWARD DECLARATIONS ===================== */

/* Terminal functions */
void terminal_setcolor(uint8_t color);
void terminal_setcursor(uint8_t row, uint8_t col);
void terminal_clear();
void terminal_scroll();
void terminal_putchar(char c);
void terminal_write(const char* str);
void terminal_write_dec(int n);
void terminal_write_hex(uint32_t n);

/* Utilities */
size_t strlen(const char* s);
int strcmp(const char* a, const char* b);
void strcpy(char* dest, const char* src);
void strcat(char* dest, const char* src);
int strncmp(const char* a, const char* b, size_t n);
char* strdup(const char* s);
void itoa(int n, char* str);

/* Memory */
void* kmalloc(uint32_t size);
void kfree(void* ptr);

/* Shell */
void shell_handle_char(char c);

/* Task scheduler */
typedef struct task task_t;
void task_add(task_t* t);
void schedule(void);

/* Timer */
void timer_tick(void);

/* Interrupts */
void isr_install(void);
void irq_install(void);

/* Keyboard */
void keyboard_handler(char c);

/* Missing utilities */
void* memset(void* ptr, int value, size_t num);
void* memcpy(void* dest, const void* src, size_t num);
char* strchr(const char* str, int c);
int atoi(const char* str);

/* Syscalls */
#define SYS_WRITE 1
#define SYS_GETUPTIME 2
#define SYS_YIELD 3
int syscall(int num, void* arg);

/* ===================== VGA ===================== */

/* VGA terminal width */
#define VGA_WIDTH 80

/* VGA terminal height */
#define VGA_HEIGHT 25

/* VGA buffer memory address */
uint16_t* vga_buffer = (uint16_t*)0xB8000;

/* Terminal row position */
uint8_t term_row = 0;

/* Terminal column position */
uint8_t term_col = 0;

/* Terminal color attribute (white on black by default) */
uint8_t term_color = 0x0F;

/* ===================== TERMINAL FUNCTIONS ===================== */

/* Set terminal color */
void terminal_setcolor(uint8_t color)
{
    /* Change the color for future prints */
    term_color = color;
}

/* Set terminal cursor position */
void terminal_setcursor(uint8_t row, uint8_t col)
{
    if (row < VGA_HEIGHT)
    {
        term_row = row;
    }

    if (col < VGA_WIDTH)
    {
        term_col = col;
    }
}

/* Clear the terminal screen */
void terminal_clear()
{
    /* Fill the entire screen with spaces */
    for (int y = 0; y < VGA_HEIGHT; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            /* Rainbow magic: vary colors by row */
            uint8_t color = (y % 15) + 1;
            vga_buffer[y * VGA_WIDTH + x] = (color << 8) | ' ';
        }
    }

    /* Reset cursor position */
    term_row = 0;
    term_col = 0;

    /* Print welcome message */
    terminal_write("Screen cleared! Rainbow magic!\n");
}

/* Scroll the terminal up by one line */
void terminal_scroll()
{
    /* Move all lines up */
    for (int y = 1; y < VGA_HEIGHT; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            vga_buffer[(y - 1) * VGA_WIDTH + x] = vga_buffer[y * VGA_WIDTH + x];
        }
    }

    /* Clear the last line */
    for (int x = 0; x < VGA_WIDTH; x++)
    {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (term_color << 8) | ' ';
    }

    /* Adjust cursor */
    if (term_row > 0)
    {
        term_row--;
    }
}

/* Print a single character to terminal */
void terminal_putchar(char c)
{
    if (c == '\n')
    {
        /* Newline: reset column and move down a row */
        term_col = 0;
        term_row++;
    }
    else if (c == '\b')
    {
        /* Backspace: move cursor back and clear character */
        if (term_col > 0)
        {
            term_col--;
        }
        vga_buffer[term_row * VGA_WIDTH + term_col] = (term_color << 8) | ' ';
    }
    else
    {
        /* Normal character: print with current color */
        vga_buffer[term_row * VGA_WIDTH + term_col] = (term_color << 8) | c;
    }

    /* Move cursor forward */
    term_col++;

    /* Handle end-of-line wrap */
    if (term_col >= VGA_WIDTH)
    {
        term_col = 0;
        term_row++;
    }

    /* Scroll if we reach bottom */
    if (term_row >= VGA_HEIGHT)
    {
        terminal_scroll();
    }
}

/* Print a string to terminal */
void terminal_write(const char* str)
{
    /* Print each character */
    for (size_t i = 0; str[i]; i++)
    {
        terminal_putchar(str[i]);
    }
}

/* Print a decimal integer */
void terminal_write_dec(int n)
{
    char buffer[12];
    int i = 0;

    if (n == 0)
    {
        buffer[i++] = '0';
        buffer[i] = '\0';
        terminal_write(buffer);
        return;
    }

    int temp = n;

    while (temp)
    {
        temp /= 10;
        i++;
    }

    buffer[i] = '\0';

    while (n)
    {
        buffer[--i] = '0' + (n % 10);
        n /= 10;
    }

    terminal_write(buffer);
}

/* Print a hexadecimal integer */
void terminal_write_hex(uint32_t n)
{
    const char* hex_chars = "0123456789ABCDEF";

    terminal_write("0x");

    for (int i = 28; i >= 0; i -= 4)
    {
        terminal_putchar(hex_chars[(n >> i) & 0xF]);
    }
}

/* ===================== UTILITIES ===================== */

/* String length */
size_t strlen(const char* s)
{
    size_t i = 0;
    while (s[i]) i++;
    return i;
}

/* String compare */
int strcmp(const char* a, const char* b)
{
    size_t i = 0;
    while (a[i] && b[i])
    {
        if (a[i] != b[i]) return a[i] - b[i];
        i++;
    }
    return a[i] - b[i];
}

/* Copy string */
void strcpy(char* dest, const char* src)
{
    size_t i = 0;
    while (src[i])
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

/* Concatenate strings */
void strcat(char* dest, const char* src)
{
    size_t i = strlen(dest);
    size_t j = 0;
    while (src[j])
    {
        dest[i + j] = src[j];
        j++;
    }
    dest[i + j] = '\0';
}

/* Compare n characters */
int strncmp(const char* a, const char* b, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        if (a[i] != b[i]) return a[i] - b[i];
        if (!a[i]) return 0;
    }
    return 0;
}

/* Duplicate string */
char* strdup(const char* s)
{
    size_t l = strlen(s);
    char* d = (char*)kmalloc(l + 1);
    strcpy(d, s);
    return d;
}

/* Integer to ASCII */
void itoa(int n, char* str)
{
    int i = 0;
    if (n == 0) { str[i++] = '0'; str[i] = '\0'; return; }
    int temp = n;
    while (temp) { temp /= 10; i++; }
    str[i] = '\0';
    while (n) { str[--i] = '0' + (n % 10); n /= 10; }
}

/* ------------------ Missing standard functions ------------------ */

/* memset implementation */
void* memset(void* ptr, int value, size_t num)
{
    unsigned char* p = (unsigned char*)ptr;
    for (size_t i = 0; i < num; i++)
        p[i] = (unsigned char)value;
    return ptr;
}

/* memcpy implementation */
void* memcpy(void* dest, const void* src, size_t num)
{
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    for (size_t i = 0; i < num; i++)
        d[i] = s[i];
    return dest;
}

/* strchr implementation */
char* strchr(const char* str, int c)
{
    while (*str)
    {
        if (*str == (char)c) return (char*)str;
        str++;
    }
    return NULL;
}

/* atoi implementation */
int atoi(const char* str)
{
    int result = 0;
    int sign = 1;
    if (*str == '-')
    {
        sign = -1;
        str++;
    }
    while (*str >= '0' && *str <= '9')
    {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result * sign;
}

/* ===================== MEMORY MANAGEMENT ===================== */

#define HEAP_SIZE 0x90000
uint8_t heap[HEAP_SIZE];
uint32_t heap_index = 0;

/* Allocate memory (bump allocator) */
void* kmalloc(uint32_t size)
{
    if (heap_index + size > HEAP_SIZE)
    {
        return 0;
    }

    void* ptr = &heap[heap_index];
    heap_index += size;
    return ptr;
}

/* Free memory (no-op) */
void kfree(void* ptr)
{
    /* Nothing happens here. Just like my emails. */
}

/* ===================== INTERRUPTS ===================== */

typedef void (*isr_t)(void);

/* Install ISRs */
void isr_install()
{
    terminal_write("Installing ISRs... sparks fly!\n");
}

/* Install IRQs */
void irq_install()
{
    terminal_write("Installing IRQs... fingers crossed!\n");
}

/* ===================== KEYBOARD ===================== */

#define KBD_BUFFER_SIZE 256
char kbd_buffer[KBD_BUFFER_SIZE];
uint32_t kbd_index = 0;

/* Handle keyboard input (fake) */
void keyboard_handler(char c)
{
    if (c == '\n')
    {
        terminal_putchar('\n');
        kbd_index = 0;
        return;
    }

    if (c == '\b')
    {
        if (kbd_index > 0)
        {
            kbd_index--;
        }
        terminal_putchar('\b');
        return;
    }

    if (kbd_index < KBD_BUFFER_SIZE - 1)
    {
        kbd_buffer[kbd_index++] = c;
        terminal_putchar(c);
    }
}

/* ===================== TASK SCHEDULER ===================== */

typedef enum { TASK_READY, TASK_RUNNING, TASK_WAITING, TASK_TERMINATED } task_state_t;

typedef struct task
{
    uint32_t id;
    uint32_t esp;
    uint32_t ebp;
    char name[16];
    task_state_t state;
    struct task* next;
} task_t;

task_t* current_task = NULL;
task_t* ready_queue = NULL;

/* Add task to ready queue */
void task_add(task_t* t)
{
    t->next = NULL;
    t->state = TASK_READY;

    if (!ready_queue)
    {
        ready_queue = t;
    }
    else
    {
        task_t* tmp = ready_queue;

        while (tmp->next)
        {
            tmp = tmp->next;
        }

        tmp->next = t;
    }
}

/* Round-robin scheduler */
void schedule()
{
    if (!ready_queue)
    {
        return;
    }

    if (!current_task)
    {
        current_task = ready_queue;
    }
    else
    {
        current_task = current_task->next ? current_task->next : ready_queue;
    }

    terminal_write("Switching to task: ");
    terminal_write(current_task->name);
    terminal_write("\n");
}

/* ===================== TIMER ===================== */

uint32_t uptime_ticks = 0;

/* Increment uptime ticks */
void timer_tick()
{
    uptime_ticks++;

    /* Print tick debug every 50 ticks */
    if (uptime_ticks % 50 == 0)
    {
        terminal_write("Timer tick: ");
        terminal_write_dec(uptime_ticks);
        terminal_write("\n");
    }
}

/* ===================== SHELL ===================== */

char shell_input[128];
uint32_t shell_index = 0;

/* Handle shell input character by character */
void shell_handle_char(char c)
{
    if (c == '\n')
    {
        terminal_putchar('\n');
        shell_input[shell_index] = '\0';

        if (strcmp(shell_input, "help") == 0)
        {
            terminal_write("Commands: help, echo, repeat, clear, uptime, date, tasks, ls, cat, echoenv\n");
        }
        else if (strncmp(shell_input, "echo ", 5) == 0)
        {
            terminal_write(shell_input + 5);
        }
        else if (strncmp(shell_input, "repeat ", 7) == 0)
        {
            int n = atoi(shell_input + 7);
            char* s = strchr(shell_input, ' ');

            if (s)
            {
                s++;

                for (int i = 0; i < n; i++)
                {
                    terminal_write(s);
                    terminal_putchar('\n');
                }
            }
        }
        else if (strcmp(shell_input, "clear") == 0)
        {
            terminal_clear();
        }
        else if (strcmp(shell_input, "uptime") == 0)
        {
            terminal_write("Uptime ticks: ");
            terminal_write_dec(uptime_ticks);
            terminal_write("\n");
        }
        else if (strcmp(shell_input, "date") == 0)
        {
            terminal_write("Date: 2025-08-27\n");
        }
        else if (strcmp(shell_input, "tasks") == 0)
        {
            task_t* t = ready_queue;

            terminal_write("Tasks:\n");

            while (t)
            {
                terminal_write("- ");
                terminal_write(t->name);
                terminal_write("\n");

                t = t->next;
            }
        }
        else if (strcmp(shell_input, "ls") == 0)
        {
            terminal_write("Files: file1.txt file2.txt file3.log\n");
        }
        else if (strncmp(shell_input, "cat ", 4) == 0)
        {
            terminal_write("File contents: Hello World!\n");
        }
        else if (strcmp(shell_input, "echoenv") == 0)
        {
            terminal_write("ENV=TinyBox-OS v5.2\n");
        }
        else
        {
            terminal_write("Unknown command. Try help.\n");
        }

        shell_index = 0;
    }
    else if (c == '\b')
    {
        if (shell_index > 0)
        {
            shell_index--;
        }
        terminal_putchar('\b');
    }
    else
    {
        shell_input[shell_index++] = c;
        terminal_putchar(c);
    }
}
int syscall(int num, void* arg)
{
    /* Dummy stub: does nothing for now */
    return 0;
}

/* ===================== KERNEL MAIN ===================== */

void kernel_main()
{
    terminal_clear();
    terminal_write("=== TinyBox-OS v5.2 Loaded ===\n");

    /* Initialize interrupts and timer */
    isr_install();
    irq_install();

    /* Create 30 dummy tasks */
    task_t* tasks[30];

    for (int i = 0; i < 30; i++)
    {
        tasks[i] = (task_t*)kmalloc(sizeof(task_t));
        tasks[i]->id = i + 1;

        for (int j = 0; j < 16; j++)
        {
            tasks[i]->name[j] = 0;
        }

        tasks[i]->name[0] = 'T';
        tasks[i]->name[1] = '0' + ((i % 10) + 1);

        task_add(tasks[i]);

        terminal_write("Task added: ");
        terminal_write(tasks[i]->name);
        terminal_write("\n");
    }

    char* msg = (char*)kmalloc(1024);

    if (msg)
    {
        terminal_write("Heap initialized successfully!\n");
    }

    memset(msg, 42, 512);
    char buffer[512];
    memcpy(buffer, msg, 512);

    terminal_write("Memory functions tested.\n");

    terminal_write("Kernel address: ");
    terminal_write_hex((uint32_t)&kernel_main);
    terminal_write("\n");

    char c = 'a';

    /* Main loop */
    while (1)
    {
        /* Fake shell input simulation */
        shell_handle_char(c);

        c++;

        if (c > 'z')
        {
            c = 'a';
        }

        /* Round-robin scheduler */
        schedule();

        /* Test syscalls */
        syscall(SYS_WRITE, "Hello from syscall!\n");
        syscall(SYS_GETUPTIME, NULL);
        syscall(SYS_YIELD, NULL);

        /* Tick timer */
        timer_tick();

        /* Delay loop */
        for (volatile int i = 0; i < 100000; i++)
        {
            /* Do nothing */
        }
    }
}
