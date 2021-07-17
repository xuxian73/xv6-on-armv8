#include "types.h"
struct context {
    uint64 elr;
    uint64 sp;

    uint64 x19;
    uint64 x20;
    uint64 x21;
    uint64 x22;
    uint64 x23;
    uint64 x24;
    uint64 x25;
    uint64 x26;
    uint64 x27;
    uint64 x28;
};


// In arm, different EL has its own TTBR,
// and share a exception vector, therefore,
// no as xv6-riscv, trapframe do not include 
// kernel_satp, kernel_trap 
struct trapframe {
    uint64  x0;
    uint64  x1;
    uint64  x2;
    uint64  x3;
    uint64  x4;
    uint64  x5;
    uint64  x6;
    uint64  x7;
    uint64  x8;
    uint64  x9;
    uint64  x10;
    uint64  x11;
    uint64  x12;
    uint64  x13;
    uint64  x14;
    uint64  x15;
    uint64  x16;
    uint64  x17;
    uint64  x18;
    uint64  x19;
    uint64  x20;
    uint64  x21;
    uint64  x22;
    uint64  x23;
    uint64  x24;
    uint64  x25;
    uint64  x26;
    uint64  x27;
    uint64  x28;
    uint64  x29;
    uint64  x30;    // link register
    uint64  sp;
    uint64  elr;    // instruction cause exception
    uint64  spsr;   // pstate
};
