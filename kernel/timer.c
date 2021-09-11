// ARM Generic Timer
#include "types.h"
#include "param.h"
#include "arm.h"
#include "mmu.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"

#define PERIOED 100 // per clock interrupt period 100ms
void isr_timer ();

struct spinlock tickslock;
uint ticks;

void set_physical_timer(int timeout_ms){
    unsigned long value, freq, cnt, cmp;
    value = 0;
    asm volatile("msr CNTP_CTL_EL0, %0" : : "r"(value) : );
    asm volatile("mrs %0, CNTFRQ_EL0" : "=r"(freq) : : );
    asm volatile("mrs %0, CNTPCT_EL0" : "=r"(cnt) : : );
    cmp = cnt + (freq / 1000) * timeout_ms;
    asm volatile("msr CNTP_CVAL_EL0, %0" : : "r"(cmp));
    value = 1;
    asm volatile("msr CNTP_CTL_EL0, %0" : : "r"(value));
}

// initialize the timer: perodical and interrupt based
void timer_init()
{
    set_physical_timer(PERIOED);
    if(cpuid() == 0)
        gic_set(PIC_TIMER, isr_timer);
}

// interrupt service routine for the timer
void isr_timer ()
{
    set_physical_timer(PERIOED);
    if(cpuid() == 0) {
        acquire(&tickslock);
        ticks++;
        wakeup(&ticks);
        release(&tickslock);
    }
}
