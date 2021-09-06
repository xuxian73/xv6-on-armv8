#include "types.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "mmu.h"
#include "arm.h"

extern char end[];
struct run {
    struct run* next;
};

struct {
    struct spinlock lock;
    struct run * freelist;
} kmem;

void
pre_free4kpt(void* va_start, void* va_end)
{
    initlock(&kmem.lock, "kmem");
    char* p = (char*)PGROUNDUP((uint64)va_start);
    for(; (void*)p < va_end; p += PGSIZE) {
        kfree(p);
    }
}

void
kinit()
{
    freerange((void*)P2V(INIT_KERNTOP), (void*)P2V(PHYSTOP));
}

void 
freerange(void *pa_start, void* pa_end)
{
    char*p;
    p = (char*)PGROUNDUP((uint64)pa_start);
    for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
        kfree(p);
}

void 
kfree(void* pa) 
{
    struct run* r;
    if (((uint64)pa % PGSIZE) != 0)
        panic("kfree");
    
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;
    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
}

void*
kalloc(void)
{
    struct run* r;
    acquire(&kmem.lock);
    r = kmem.freelist;
    if (r)
        kmem.freelist = r->next;
    release(&kmem.lock);
    
    if(r)
        memset((char*)r, 0, PGSIZE);
    return (void*)r; 
}