#include "../device/arm_virt.h"

#define DAIF_I (1 << 7)
static inline uint64
r_CurrentEL()
{
    uint64 x;
    asm("mrs %0, CurrentEL" : "=r"(x) : :);
    return x;
}

static inline void
w_CPACR_EL1(uint64 x)
{
    asm("msr CPACR_EL1, %0": : "r"(x) :); 
}

static inline void 
w_TTBR0_EL1(uint64 x)
{
    asm("msr TTBR0_EL1, %0": : "r"(x) :);
}

static inline void 
w_TTBR1_EL1(uint64 x)
{
    asm("msr TTBR1_EL1, %0": : "r"(x) :);
}

// close local interrupt
static inline void
cli()
{
    asm("msr DAIFSET, #2": : :);
}

// open local interrupt
static inline void
sti()
{
    asm("msr DAIFCLR, #2": : :);
}