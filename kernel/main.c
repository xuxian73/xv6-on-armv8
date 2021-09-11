#include "types.h"
#include "defs.h"
#include "param.h"
#include "arm.h"
#include "spinlock.h"
#include "proc.h"
#include "memlayout.h"
#include "mmu.h"

extern char end[];
extern struct cpu cpus[];
volatile static int started = 0;
void 
kmain()
{
    if(cpuid() == 0){
        uart_init ((void*)P2V(UARTBASE));
        consoleinit();
        printfinit();
        printf("\n");
        printf("xv6 kernel is booting\n");
        printf("\n");
        pre_free4kpt(&end, (void*)P2V(INIT_KERNTOP));
        page_init();
        kinit();
        procinit();
        gic_init();
        uart_enable_rx();
        binit();
        iinit();
        virtio_disk_init();
        timer_init();
        fileinit();

        userinit();
        __sync_synchronize();
        started = 1;
    } else {
        while(started == 0)
            ;
        __sync_synchronize();
        printf("hart %d starting\n", cpuid());
        //gichartinit(cpuid());
        timer_init();
        gichartinit();
    }
    scheduler();
}