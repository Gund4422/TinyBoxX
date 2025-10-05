#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

/* -------------------- Tiny Stdlib -------------------- */
size_t strlen(const char* s){size_t i=0;while(s[i])i++;return i;}
int strcmp(const char* a,const char* b){size_t i=0;while(a[i]&&b[i]){if(a[i]!=b[i])return a[i]-b[i];i++;}return a[i]-b[i];}
void strcpy(char* d,const char* s){size_t i=0;while(s[i]){d[i]=s[i];i++;}d[i]='\0';}
void strcat(char* d,const char* s){size_t i=strlen(d),j=0;while(s[j]){d[i+j]=s[j];j++;}d[i+j]='\0';}
int atoi(const char* s){int r=0,sig=1;if(*s=='-'){sig=-1;s++;}while(*s>='0'&&*s<='9'){r=r*10+(*s-'0');s++;}return r*sig;}
void itoa(int n,char* b){int i=0,t=n;if(!n){b[i++]='0';b[i]='\0';return;}while(t){t/=10;i++;}b[i]='\0';while(n){b[--i]='0'+(n%10);n/=10;}}
void* memset(void* p,int v,size_t n){unsigned char* c=(unsigned char*)p;for(size_t i=0;i<n;i++)c[i]=(unsigned char)v;return p;}
void* memcpy(void* d,const void* s,size_t n){unsigned char* dd=(unsigned char*)d;const unsigned char* ss=(const unsigned char*)s;for(size_t i=0;i<n;i++)dd[i]=ss[i];return d;}
char* strchr(const char* s,int c){while(*s){if(*s==(char)c)return (char*)s;s++;}return 0;}

/* -------------------- VGA Terminal -------------------- */
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
uint16_t* vga=(uint16_t*)0xB8000;
uint8_t term_row=0,term_col=0,term_color=0x0F;

void terminal_setcolor(uint8_t color){term_color=color;}
void terminal_setcursor(uint8_t r,uint8_t c){if(r<VGA_HEIGHT)term_row=r;if(c<VGA_WIDTH)term_col=c;}
void terminal_scroll(){for(int y=1;y<VGA_HEIGHT;y++)for(int x=0;x<VGA_WIDTH;x++)vga[(y-1)*VGA_WIDTH+x]=vga[y*VGA_WIDTH+x];for(int x=0;x<VGA_WIDTH;x++)vga[(VGA_HEIGHT-1)*VGA_WIDTH+x]=(term_color<<8)|' ';if(term_row>0)term_row--;}
void terminal_putchar(char c){if(c=='\n'){term_col=0;term_row++;}else if(c=='\b'){if(term_col>0)term_col--;vga[term_row*VGA_WIDTH+term_col]=(term_color<<8)|' ';}else vga[term_row*VGA_WIDTH+term_col]=(term_color<<8)|c;term_col++;if(term_col>=VGA_WIDTH){term_col=0;term_row++;}if(term_row>=VGA_HEIGHT)terminal_scroll();}
void terminal_write(const char* s){for(size_t i=0;s[i];i++)terminal_putchar(s[i]);}
void terminal_write_dec(int n){char b[12];itoa(n,b);terminal_write(b);}
void terminal_write_hex(uint32_t n){const char* h="0123456789ABCDEF";terminal_write("0x");for(int i=28;i>=0;i-=4)terminal_putchar(h[(n>>i)&0xF]);}
void terminal_clear(){for(int y=0;y<VGA_HEIGHT;y++)for(int x=0;x<VGA_WIDTH;x++)vga[y*VGA_WIDTH+x]=((y%15+1)<<8)|' ';term_row=term_col=0;terminal_write("Screen cleared! Rainbow magic!\n");}

/* -------------------- Memory -------------------- */
#define HEAP_SIZE 0x90000
uint8_t heap[HEAP_SIZE]; uint32_t heap_index=0;
void* kmalloc(uint32_t s){if(heap_index+s>HEAP_SIZE)return 0;void* p=&heap[heap_index];heap_index+=s;return p;}
void kfree(void* ptr){}

/* -------------------- Tasks -------------------- */
typedef enum{TASK_READY,TASK_RUNNING,TASK_WAITING,TASK_TERMINATED}task_state_t;
typedef struct task{uint32_t id;uint32_t esp,ebp;char name[16];task_state_t state;struct task* next;}task_t;
task_t* current_task=0,*ready_queue=0;
void task_add(task_t* t){t->next=0;t->state=TASK_READY;if(!ready_queue)ready_queue=t;else{task_t* tmp=ready_queue;while(tmp->next)tmp=tmp->next;tmp->next=t;}}
void schedule(){if(!ready_queue)return;if(!current_task)current_task=ready_queue;else current_task=current_task->next?current_task->next:ready_queue;terminal_write("Switching to task: ");terminal_write(current_task->name);terminal_write("\n");}

/* -------------------- Timer -------------------- */
uint32_t uptime_ticks=0;
void timer_tick(){uptime_ticks++;if(uptime_ticks%50==0){terminal_write("Timer tick: ");terminal_write_dec(uptime_ticks);terminal_write("\n");}}

/* -------------------- Shell -------------------- */
char shell_input[128]; uint32_t shell_index=0;
void shell_handle_char(char c){
    if(c=='\n'){terminal_putchar('\n');shell_input[shell_index]='\0';
        if(strcmp(shell_input,"help")==0)terminal_write("Commands: help, echo, repeat, clear, uptime, date, tasks, ls, cat, echoenv\n");
        else if(strncmp(shell_input,"echo ",5)==0)terminal_write(shell_input+5);
        else if(strncmp(shell_input,"repeat ",7)==0){int n=atoi(shell_input+7);char* s=strchr(shell_input,' ');if(s){s++;for(int i=0;i<n;i++){terminal_write(s);terminal_putchar('\n');}}}
        else if(strcmp(shell_input,"clear")==0)terminal_clear();
        else if(strcmp(shell_input,"uptime")==0){terminal_write("Uptime ticks: ");terminal_write_dec(uptime_ticks);terminal_write("\n");}
        else if(strcmp(shell_input,"date")==0)terminal_write("Date: 2025-08-27\n");
        else if(strcmp(shell_input,"tasks")==0){task_t* t=ready_queue;terminal_write("Tasks:\n");while(t){terminal_write("- ");terminal_write(t->name);terminal_write("\n");t=t->next;}}
        else if(strcmp(shell_input,"ls")==0)terminal_write("Files: file1.txt file2.txt file3.log\n");
        else if(strncmp(shell_input,"cat ",4)==0)terminal_write("File contents: Hello World!\n");
        else if(strcmp(shell_input,"echoenv")==0)terminal_write("ENV=TinyBox-OS v5.2\n");
        else terminal_write("Unknown command. Try help.\n");
        shell_index=0;}
    else if(c=='\b'){if(shell_index>0)shell_index--;terminal_putchar('\b');}
    else{shell_input[shell_index++]=c;terminal_putchar(c);}
}

/* -------------------- Keyboard -------------------- */
#define KBD_BUFFER_SIZE 256
char kbd_buffer[KBD_BUFFER_SIZE]; uint32_t kbd_index=0;
void keyboard_handler(char c){if(c=='\n'){terminal_putchar('\n');kbd_index=0;return;}if(c=='\b'){if(kbd_index>0)kbd_index--;terminal_putchar('\b');return;}if(kbd_index<KBD_BUFFER_SIZE-1){kbd_buffer[kbd_index++]=c;terminal_putchar(c);}}

/* -------------------- ISRs & IRQs -------------------- */
void isr_install(){terminal_write("Installing ISRs... sparks fly!\n");}
void irq_install(){terminal_write("Installing IRQs... fingers crossed!\n");}

/* -------------------- Syscalls -------------------- */
#define SYS_WRITE 1
#define SYS_GETUPTIME 2
#define SYS_YIELD 3
int syscall(int num, void* arg){return 0;}

/* -------------------- Kernel -------------------- */
void kernel_main(){
    terminal_clear(); terminal_write("=== TinyBox-OS v5.2 Loaded ===\n");
    isr_install(); irq_install();

    task_t* tasks[30];
    for(int i=0;i<30;i++){
        tasks[i]=(task_t*)kmalloc(sizeof(task_t));
        tasks[i]->id=i+1;
        for(int j=0;j<16;j++)tasks[i]->name[j]=0;
        tasks[i]->name[0]='T'; tasks[i]->name[1]='0'+((i%10)+1);
        task_add(tasks[i]);
        terminal_write("Task added: ");terminal_write(tasks[i]->name);terminal_write("\n");
    }

    char* msg=(char*)kmalloc(1024);
    if(msg){terminal_write("Heap initialized successfully!\n");}
    memset(msg,42,512); char buffer[512]; memcpy(buffer,msg,512);
    terminal_write("Memory functions tested.\n");
    terminal_write("Kernel address: "); terminal_write_hex((uint32_t)&kernel_main); terminal_write("\n");

    char c='a';
    while(1){
        shell_handle_char(c); c++; if(c>'z')c='a';
        schedule(); syscall(SYS_WRITE,"Hello from syscall!\n"); syscall(SYS_GETUPTIME,0); syscall(SYS_YIELD,0);
        timer_tick();
        for(volatile int i=0;i<100000;i++);
    }
}

