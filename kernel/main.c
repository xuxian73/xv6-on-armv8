#include "types.h"
#include "defs.h"
#include "param.h"
#include "arm.h"
#include "spinlock.h"
#include "proc.h"
#include "memlayout.h"
#include "mmu.h"

extern char end[];
extern struct cpu* cpu;
extern struct cpu CPUS[];
void 
kmain()
{
    cpu = CPUS;
    pre_free4kpt(&end, (void*)P2V(INIT_KERNTOP));
    page_init();
    kinit();
    printfinit();
    
}