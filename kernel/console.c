// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "arm.h"

#define BACKSPACE 0x100
#define C(x)    ((x) - '@')  // Control-x

static int panicked = 0;
// send one character to the uart
// called by printf, and to echo input characters,
// but not from write()
void 
consputc(int c)
{
    if(panicked) {
        cli();
        while(1);
    }
    if (c == BACKSPACE) {
        uartputc_sync('\b');
        uartputc_sync(' ');
        uartputc_sync('\b');
    } else {
        uartputc_sync(c);
    }
}

struct {
  struct spinlock lock;
  
  // input
#define INPUT_BUF 128
  char buf[INPUT_BUF];
  uint r;  // Read index
  uint w;  // Write index
  uint e;  // Edit index
} cons;

// user write()s to the console go here.
int
consolewrite(int user_src, uint64 src, int n)
{
    int i;
    acquire(&cons.lock);
    for(i = 0; i < n; ++i) {
        char c;
        if(either_copyin(&c, user_src, src+i, 1) == -1)
            break;
        uartputc(c);
    }
    release(&cons.lock);
    return i;
}

// user read()s from the console go here
// copy a whole input line to dst
// user_dst indicate whether dst is
// a kernel address 
int 
consoleread(int user_dst, uint64 dst, int n)
{
    uint target;
    int c;
    char cbuf;
    target = n;
    acquire(&cons.lock);
    while(n > 0) {
        while(cons.r == cons.w) {
            if(myproc()->killed) {
                release(&cons.lock);
                return -1;
            }
            sleep(&cons.r, &cons.lock);
        }

        c = cons.buf[cons.r++ %INPUT_BUF];

        if(c == C('D')) {   // end-of-file
            if(n < target) {
                // Save ^D for next time, to make sure
                // caller gets a 0-byte result.
                cons.r--;
            }
            break;
        }
        
        //copy the input byte to the user-space buf
        cbuf = c;
        if(either_copyout(user_dst, dst, &cbuf, 1) == -1)
            break;
        
        dst++;
        --n;
        if (c == '\n'){
            // a whole line has arrived, return to
            // the user-level read()
            break;
        }
    }
    release(&cons.lock);

    return target - n;
}

// the console input interrupt handler.
// uartintr() calls this for input character.
// do erase/kill processing, append to cons.buf
// wake up consoleread() if a wholeline has arrived.
void
consoleintr(int c)
{
    acquire(&cons.lock);
    switch(c) {
        case C('P'): // Print process list
            procdump();
            break;
        case C('U'):    // Kill line
            while(cons.e != cons.w &&
            cons.buf[(cons.e-1) % INPUT_BUF] != '\n') {
                cons.e--;
                consputc(BACKSPACE);
            }
            break;
        case C('H'):   // Backspace
        case '\x7f':
            if(cons.e != cons.w) {
                cons.e--;
                consputc(BACKSPACE);
            }
            break;
        default:
            if(c != 0 && cons.e-cons.r < INPUT_BUF) {
                c = (c == '\r') ? '\n' : c;
                // echo back to the user.
                consputc(c);
                // store for consumption by consoleread().
                cons.buf[cons.e++ % INPUT_BUF] = c;

                if(c == '\n' || c == C('D') || cons.e == cons.r+INPUT_BUF) {
                    cons.w = cons.e;
                    wakeup(&cons.r);
                }
            }
            break;
    }

    release(&cons.lock);
}

void
consoleinit(void)
{
  initlock(&cons.lock, "cons");

  // connect read and write system calls
  // to consoleread and consolewrite.
  devsw[CONSOLE].read = consoleread;
  devsw[CONSOLE].write = consolewrite;
}
