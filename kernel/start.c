#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "arm.h"
#include "defs.h"
#include "mmu.h"

void uart_putc(int c)
{
    volatile uint8* uart0 = (uint8*)UARTBASE;
    *uart0 = c;
}

void _puts(char* s)
{
    while (*s != '\0') {
        uart_putc(*s);
        ++s;
    }
}

void _puts_int(uint num) {
    int i;
    char* arr = "0123456789ABCDEF";
    for (i = sizeof(num) * 8 - 4; i >=0; i -= 4) {
        uart_putc(arr[(num >> i) & 0xf]);
    }
}

extern void jump_stack();
extern void kmain();
extern void* vectors;
extern uint64 _kernel_pgtbl;
extern uint64 _user_pgtbl;
extern uint64 _K_l2_pgtbl;
extern uint64 _U_l2_pgtbl;
extern void *edata_entry;
extern void *edata;
extern void *end;
pagetable_t kernel_pgtbl = &_kernel_pgtbl;
pagetable_t user_pgtbl = &_user_pgtbl;

void init_PGD_table() 
{
    uint32 i;
    
    uint64 pgd_entry;
    for (i = 0; i < 4; ++i) {
        pgd_entry = (uint64)&_K_l2_pgtbl + 4096 * i;
        pgd_entry |= ENTRY_VALID | ENTRY_TABLE;
        kernel_pgtbl[i] = pgd_entry;
    }
    for (i = 0; i < 4; ++i) {
        pgd_entry = (uint64)&_U_l2_pgtbl + 4096 * i;
        pgd_entry |= ENTRY_VALID | ENTRY_TABLE;
        user_pgtbl[i] = pgd_entry;
    }
}

//boot virtual memory map
void bvmmap(uint64 va, uint64 pa, uint sz, int dev_mem)
{
    uint64 pde;
    pagetable_t l2;
    uint64 end = va + sz;
    for (;;) {
        pde = pa & PMD_MASK;
        if (!dev_mem) {
            // normal memory
            pde |= ACCESS_FLAG | SH_IN_SH | AP_0 | ENTRY_NS | MEM_ATTR_IDX(4) | ENTRY_BLOCK | ENTRY_VALID | UXN;
        } else {
            // device memory
            pde |= ACCESS_FLAG | AP_0 | MEM_ATTR_IDX(0) | ENTRY_BLOCK | ENTRY_VALID;
        }
        l2 = (pagetable_t)(kernel_pgtbl[PGD_IDX(va)] & PG_ADDR_MASK);
        l2[PMD_IDX(va)] = pde;
        l2 = (pagetable_t)(user_pgtbl[PGD_IDX(va)] & PG_ADDR_MASK);
        l2[PMD_IDX(va)] = pde;
        va += PMD_SZ;
        pa += PMD_SZ;
        if (va >= end)
            break;
    }
}

void cpu_setup()
{
    uint64 x;
    // x = r_CurrentEL() >> 2;
    // x &= 0x3;
    // _puts("CurrentEL: EL");
    // _puts_int(x);
    // _puts("\n");
    // x = cpuid();
    // _puts_int(x);
    //flush instruction cache
    asm("ic iallu": : :);
    //invalidate tlb
    asm("tlbi vmalle1is" : : :);
    //data sync barrier
    asm("dsb sy" : : :);
    //disable trap in on FP/SIMD instruction
    w_CPACR_EL1(3 << 20);
    //disable monitor debug
    asm("msr MDSCR_EL1, xzr" : : :);
    //set memory attribute indirection register
    // DEVICE_nGnRnE    0 
    // DEVICE_nGnRE     1 
    // DEVICE_GRE       2 
    // NORMAL_NC        3 
    // NORMAL           4
    x = (uint64)0xFF440C0400;
    asm("msr MAIR_EL1, %0": : "r"(x) :);
    asm("isb");
    // set exception vector base address register
    x = (uint64)&vectors;
    asm("msr VBAR_EL1, %0": : "r"(x) :);
    //set translation control register
    x = (uint64)0x34B5203520;
    asm("msr TCR_EL1, %0": : "r" (x):);
    asm("isb": : :);
    //set translation table base register 1 (kernel)
    w_TTBR1_EL1((uint64)kernel_pgtbl);
    //set translation table base register 2 (user)
    w_TTBR0_EL1((uint64)user_pgtbl);
    asm("isb": : :);
    //set system control register
    asm("mrs %0, SCTLR_EL1": "=r"(x): :); //0x0000000000c50838
    x = x | 0x01;
    asm("msr SCTLR_EL1, %0": : "r"(x):);
    asm("isb": : :);
    // _puts("System Configure Completed...\n\n");
}

void init_boot_pgtbl() 
{
    bvmmap((uint64)PHYSTART, (uint64)PHYSTART, KERN_SZ, 0);
    bvmmap((uint64)KERNBASE + (uint64)PHYSTART, (uint64)PHYSTART, KERN_SZ, 0);
    bvmmap((uint64)UARTBASE, (uint64)UARTBASE, DEV_SZ, 1);
    bvmmap((uint64)VIRTIOBASE, (uint64)VIRTIOBASE, DEV_SZ, 1);
    bvmmap((uint64)KERNBASE + (uint64)GICBASE, (uint64)GICBASE, DEV_SZ, 1);
    bvmmap((uint64)KERNBASE + (uint64)UARTBASE, (uint64)UARTBASE, DEV_SZ, 1);
    bvmmap((uint64)KERNBASE + (uint64)VIRTIOBASE, (uint64)VIRTIOBASE, DEV_SZ, 1);
}

void clear_bss (void)
{
    memset(&edata, 0x00, &end-&edata);
}

volatile static int set_pgtbl_done = 0;
void 
start(void) 
{
    //_puts("Starting...\n");
    if (cpuid() == 0) {
        init_PGD_table();
        init_boot_pgtbl();
        set_pgtbl_done = 1;
        __sync_synchronize();
    } else {
        while(set_pgtbl_done == 0)
            ;
    }

    cpu_setup();

    jump_stack();

    // clear_bss();

    //_puts("Starting kernel...\n");

    kmain();
}