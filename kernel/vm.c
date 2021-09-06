#include "param.h"
#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "mmu.h"
#include "arm.h"

pagetable_t kpgdir;
extern uint64 _kernel_pgtbl;

pte_t *
walk(pagetable_t pagetable, uint64 va, int alloc)
{
    pte_t *pgd = &pagetable[PGD_IDX((uint64)va)];
    if (*pgd & (ENTRY_VALID | ENTRY_TABLE))
    {
        pagetable = (pagetable_t)(P2V(PG_ADDR_MASK & *pgd));
    }
    else
    {
        if (!alloc || (pagetable = (pagetable_t)kalloc()) == 0)
            return 0;
        *pgd = V2P(pagetable) | ENTRY_TABLE | ENTRY_VALID;
    }

    pte_t *pmd = &pagetable[PMD_IDX((uint64)va)];
    if (*pmd & (ENTRY_TABLE | ENTRY_VALID))
    {
        pagetable = (pagetable_t)(P2V(PG_ADDR_MASK & *pmd));
    }
    else
    {
        if (!alloc || (pagetable = (pagetable_t)kalloc()) == 0)
            return 0;
        memset(pagetable, 0, PGSIZE);
        *pmd = V2P(pagetable) | ENTRY_VALID | ENTRY_TABLE;
    }
    return &pagetable[PTE_IDX(va)];
}

uint64
walkaddr(pagetable_t pagetable, uint64 va)
{
    pte_t *pte;
    uint64 pa;

    pte = walk(pagetable, va, 0);
    if (pte == 0)
        return 0;
    if ((*pte & (ENTRY_PAGE | ENTRY_VALID)) == 0)
        return 0;
    pa = *pte & PG_ADDR_MASK;
    return pa;
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned. Returns 0 on success, -1 if walk() couldn't
// allocate a needed page-table page.
// AP: access permission
int mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int AP)
{
    uint64 a, last;
    pte_t *pte;
    a = PGROUNDDOWN(va);
    last = PGROUNDDOWN(va + size - 1);
    for (;;)
    {
        if ((pte = walk(pagetable, a, 1)) == 0)
            return -1;
        if (*pte & (ENTRY_PAGE | ENTRY_VALID))
            panic("remap");
        *pte = pa | ACCESS_FLAG | SH_IN_SH | AP | ENTRY_NS | MEM_ATTR_IDX(4) | ENTRY_PAGE | ENTRY_VALID;
        if (a == last)
            break;
        a += PGSIZE;
        pa += PGSIZE;
    }
    return 0;
}

void page_init(void)
{
    mappages(&_kernel_pgtbl, P2V(INIT_KERNTOP), PHYSTOP - INIT_KERNTOP, INIT_KERNTOP, AP_1);
    asm("tlbi vmalle1" :::);
    //vnalle1is?
}

// Remove npages of mappings starting from va. va must be
// page-aligned. The mappings must exist.
// Optionally free the physical memory.
void uvmunmap(pagetable_t pagetable, uint64 va, uint64 npages, int do_free)
{
    uint64 a;
    pte_t *pte;

    if ((va % PGSIZE) != 0)
        panic("uvmunmap: not aligned");

    for (a = va; a < va + npages * PGSIZE; a += PGSIZE)
    {
        if ((pte = walk(pagetable, a, 0)) == 0)
            panic("uvmunmap: walk");
        if ((*pte & ENTRY_VALID) == 0)
            panic("uvmunmap: not mapped");
        if ((*pte & ENTRY_PAGE) == 0)
            panic("uvmunmap: not a page");
        if (do_free)
        {
            uint64 pa = (*pte) & PG_ADDR_MASK;
            kfree((void *)pa);
        }
        *pte = 0;
    }
}

// create an empty user page table.
// returns 0 if out of memory.
pagetable_t
uvmcreate()
{
    pagetable_t pagetable;
    pagetable = (pagetable_t)kalloc();
    if (pagetable == 0)
        return 0;
    memset(pagetable, 0, PGSIZE);
    return pagetable;
}

// Load the user initcode into address 0 of pagetable,
// for the very first process.
// sz must be less than a page.
void uvminit(pagetable_t pagetable, uchar *src, uint sz)
{
    char *mem;
    if (sz >= PGSIZE)
        panic("uvminit: more than a page");
    mem = kalloc();
    memset(mem, 0, PGSIZE);
    mappages(pagetable, 0, PGSIZE, (uint64)V2P(mem), AP_1);
    memmove(mem, src, sz);
}

uint64
uvmalloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz)
{
    char *mem;
    uint64 a;
    if (newsz < oldsz)
        return oldsz;
    oldsz = PGROUNDUP(oldsz);
    for (a = oldsz; a < newsz; a += PGSIZE)
    {
        mem = kalloc();
        if (mem == 0)
        {
            uvmdealloc(pagetable, a, oldsz);
            return 0;
        }
        memset(mem, 0, PGSIZE);
        if (mappages(pagetable, a, PGSIZE, (uint64)V2P(mem), AP_1) != 0)
        {
            kfree(mem);
            uvmdealloc(pagetable, a, oldsz);
            return 0;
        }
    }
    return newsz;
}

uint64
uvmdealloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz)
{
    if (newsz >= oldsz)
        return oldsz;
    if (PGROUNDUP(newsz) < PGROUNDUP(oldsz))
    {
        int npages = (PGROUNDUP(oldsz) - PGROUNDUP(newsz)) / PGSIZE;
        uvmunmap(pagetable, PGROUNDUP(newsz), npages, 1);
    }
    return newsz;
}

void freewalk(pagetable_t pagetable)
{
    pagetable_t pmdtb;
    pagetable_t ptetb;
    int i,j,k;
    for (i = 0; i < 4; ++i)
    {
        pte_t pgd = pagetable[i];
        if ((pgd & ENTRY_VALID) && (pgd & ENTRY_TABLE))
        {
            pmdtb = (pagetable_t)(pgd & PG_ADDR_MASK);
            for (j = 0; j < 512; ++j)
            {
                pte_t pmd = pmdtb[j];
                if ((pmd & ENTRY_VALID) && (pmd & ENTRY_TABLE))
                {
                    ptetb = (pagetable_t)(pmd & PG_ADDR_MASK);
                    for (k = 0; k < 512; ++k)
                    {
                        pte_t pte = ptetb[k];
                        if (pte & (ENTRY_VALID | ENTRY_PAGE))
                        {
                            panic("freewalk: leaf");
                        }
                    }
                    kfree((void *)ptetb);
                }
            }
            kfree((void *)pmdtb);
        }
    }
    kfree((void *)pagetable);
}

// Free user memory pages,
// then free page-table pages.
void uvmfree(pagetable_t pagetable, uint64 sz)
{
    if (sz > 0)
        uvmunmap(pagetable, 0, PGROUNDUP(sz) / PGSIZE, 1);
    freewalk(pagetable);
}

int uvmcopy(pagetable_t old, pagetable_t new, uint64 sz)
{
    pte_t *pte;
    uint64 pa, i;
    uint flags;
    char *mem;

    for (i = 0; i < sz; i += PGSIZE)
    {
        if ((pte = walk(old, i, 0)) == 0)
            panic("uvmcopy: pte should exist");
        if ((*pte & ENTRY_VALID) == 0)
            panic("uvmcopy: page not present");
        pa = *pte & PG_ADDR_MASK;
        flags = (*pte) & (0x3 << 6);
        if ((mem = kalloc()) == 0)
        {
            goto bad;
        }
        memmove(mem, (char *)P2V(pa), PGSIZE);
        if (mappages(new, i, PGSIZE, V2P(mem), flags) != 0)
        {
            kfree(mem);
            goto bad;
        }
    }
    return 0;
bad:
    uvmunmap(new, 0, i / PGSIZE, 1);
    return 0;
}

void uvmclear(pagetable_t pagetable, uint64 va)
{
    pte_t *pte;
    pte = walk(pagetable, va, 0);
    if (pte == 0)
        panic("uvmclear");
    *pte &= ~(0x03 << 6);
}

// Switch to TTBR0
void 
uvmswitch(struct proc* p)
{
    uint64 x;
    push_off();
    if(p->pagetable == 0) {
        panic("uvmswitch: no pagetable");
    }
    x = (uint64)V2P(p->pagetable);
    asm("MSR TTBR0_EL1, %0" : : "r"(x) :);
    asm ("tlbi vmalle1" : : :);
    pop_off();
}

// Copy from kernel to user.
// Copy len bytes from src to virtual address dstva in a given page table.
// Return 0 on success, -1 on error.
int copyout(pagetable_t pagetable, uint64 dstva, char *src, uint64 len)
{
    uint64 n, va0, pa0;
    while (len > 0)
    {
        va0 = PGROUNDDOWN(dstva);
        pa0 = walkaddr(pagetable, va0);
        if (pa0 == 0)
            return -1;
        n = PGSIZE - (dstva - va0);
        if (n > len)
            n = len;
        memmove((void *)P2V(pa0 + (dstva - va0)), src, n);
    }
    len -= n;
    src += n;
    dstva = va0 + PGSIZE;
    return 0;
}

int 
copyin(pagetable_t pagetable, char *dst, uint64 srcva, uint64 len)
{
    uint64 n, va0, pa0;

    while (len > 0)
    {
        va0 = PGROUNDDOWN(srcva);
        pa0 = walkaddr(pagetable, va0);
        if (pa0 == 0)
            return -1;
        n = PGSIZE - (srcva - va0);
        if (n > len)
            n = len;
        memmove(dst, (void *)P2V(pa0 + (srcva - va0)), n);

        len -= n;
        dst += n;
        srcva = va0 + PGSIZE;
    }
    return 0;
}

int
copyinstr(pagetable_t pagetable, char*dst, uint64 srcva, uint64 max)
{
    uint64 n, va0, pa0;
    int got_null = 0;

    while(got_null == 0 && max > 0){
        va0 = PGROUNDDOWN(srcva);
        pa0 = walkaddr(pagetable, va0);
        if(pa0 == 0)
            return -1;
        n = PGSIZE - (srcva - va0);
        if(n > max)
            n = max;
        char *p = (char*)P2V(pa0 + (srcva - va0));
        while(n > 0) {
            if(*p == '\0') {
                *dst = '\0';
                got_null = 1;
                break;
            } else { 
                *dst = *p;
            }
            --n;
            --max;
            p++;
            dst++;
        }

        srcva = va0 + PGSIZE;
    }
    if(got_null) {
        return 0;
    } else {
        return -1;
    }
}