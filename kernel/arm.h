#include "../device/arm_virt.h"
#include "types.h"

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

typedef uint64 pte_t;
typedef uint64 pde_t;
typedef uint64 *pagetable_t;