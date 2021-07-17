#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "defs.h"
#include "mmu.h"
struct run {
    struct run* next;
};

struct {
    struct spinlock lock;
    struct run * freelist;
} kmem;

void
kinit()
{
    initlock(&kmem.lock, "kmem");
    
}

void 
freerange(void *pa_start, void* pa_end)
{
    char*p;
    p = (char*)PGROUNDUP((uint64)pa_start);
    for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
        kfree(p);
}

void kfree(void* pa) 
{
    
}
