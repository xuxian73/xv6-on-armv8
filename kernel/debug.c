#include "types.h"
#include "defs.h"
#include "arm.h"


uint64 debug_current_el() {
  uint64 current_el;
  asm volatile("mrs %0, CurrentEL\n\t" : "=r" (current_el) : :);
  return current_el;
}

int
debug_intr_get(void)
{
    uint64 x;
    asm("mrs %0, DAIF": "=r"(x) : :);
    return !(x & DAIF_I);
}