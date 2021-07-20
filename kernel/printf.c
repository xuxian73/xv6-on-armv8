#include "types.h"
#include "defs.h"
#include "spinlock.h"

volatile int panicked = 0;

static struct {
    struct spinlock lock;
    int locking;
} pr;

void
panic(char* s)
{
    pr.locking = 0;
    _puts("panic: ");
    _puts(s);
    _puts("\n");
    panicked = 1;
    for(;;)
        ;
}

void printfinit(void)
{
    initlock(&pr.lock, "pr");
    pr.locking = 1;
}

