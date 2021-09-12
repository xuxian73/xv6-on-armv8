#include "types.h"
#include "defs.h"
#include "param.h"
#include "arm.h"
#include "memlayout.h"
#include "mmu.h"

/* Distributor register map */
#define GICD_CTLR       0x000   // Distributor Control Register
#define GICD_TYPER      0x004   // Interrupt Controller Type Register
#define GICD_IIDR       0x008   // Distributor Implementer Identification Register
#define GICD_IGROUPRn   0x080   // Interrupt Group Register
#define GICD_ISENABLERn 0x100   // Interrupt Set-Enable Registers
#define GICD_ICENABLERn 0x180   // Interrupt Clear-Enable Registers
#define GICD_ISPENDRn   0x200   // Interrupt Set-pending Registers
#define GICD_ICPENDRn   0x280   // Interrupt Clear-pending Registers
#define GICD_ISACTIVERn 0x300   // Interrupt Set-active 
#define GICD_ICACTIVERn 0x380   // Interrupt Clear-active
#define GICD_IPRIORITYR 0x400   // Interrupt Priority Registers
#define GICD_ITARGETSRn 0x800   // Interrupt Processor Targets Registers
#define GICD_ICFGRn     0xc00   // Interrupt Configuration Registers

/* CPU interface register map */
#define GICC_CTLR       0x000   // CPU interface control regsiter
#define GICC_PMR        0x004   // Interrupt Priority Mask register
#define GICC_BPR        0x008   // Binary Point Register
#define GICC_IAR        0x00c   // Interrupt Acknowledge Register
#define GICC_EOIR       0x010   // End of Interrupt Register
#define GICC_RRR		0x014   // Running Priority Register
#define GICC_HPPIR		0x018   // Highest Priority Pending Interrupt Register
/* Aliased Register */
#define GICC_ABPR		0x01C  
#define GICC_AIAR		0x020
#define GICC_AEOIR		0x024
#define GICC_AHPPIR		0x028

#define GICC_APR		0x0D0   // Active Priority Registers
#define GICC_NSAPR		0x0E0   
#define GICC_IIDR		0x0FC   // CPU Interface Identification Register
#define GICC_DIR		0x1000  // Deactive Interrupt Register

#define GICD_reg(r)     ((volatile uint*)(KERNBASE + GICBASE + r))
#define GICC_reg(r)     ((volatile uint*)(KERNBASE + GICBASE + 0x10000 + r))

static ISR isrs[64];

static void gic_dist_switch(uint64 num, int enable)
{
    uint val;
    uint bitoff = num % 32;
    val = *GICD_reg(GICD_ISENABLERn + (num / 32) * 4);
    if(enable)
        val |= 1 << bitoff;
    else
        val &= ~(1 << bitoff);
    *GICD_reg(GICD_ISENABLERn + (num / 32) * 4) = val;
}

static void gic_dist_config(uint64 num, int edge, int target)
{
    uint val, bitoff;
    /* configure */
    bitoff = (num % 16) * 2;
    val = *GICD_reg(GICD_ICFGRn + (num / 16) * 4);
    val &= (~0x3 << bitoff);
    if(edge)
        val |= 0x2 << bitoff;
    *GICD_reg(GICD_ICFGRn + (num / 16) * 4) = val;

    /* enable*/
    gic_dist_switch(num, 1);

    /* group: default 0 */
    /* group 0: FIQ, group 1: IRQ */

    /* target CPU */
    bitoff = (num % 4) * 8;
    val = *GICD_reg(GICD_ITARGETSRn + (num / 4)* 4);
    val |= target << bitoff;
    *GICD_reg(GICD_ITARGETSRn + (num / 4) * 4) = val;
}

static void default_isr ()
{
    printf("unhandled interrupt\n");
}

void gichartinit(int hart){
    *GICC_reg(GICC_PMR) = 0xf;
    *GICC_reg(GICC_CTLR) |= 1;
    /* TIMER is private PPI to core */
    gic_dist_config(PIC_TIMER, 0, 1 << hart);
}

void gic_init(){
    int i;
    for(i = 0; i < 32; ++i)
        isrs[i] = default_isr;
    *GICC_reg(GICC_PMR) = 0xf;
    gic_dist_config(PIC_TIMER, 0, 1);
    gic_dist_config(PIC_UART0 + 32, 0, 0x11);
    gic_dist_config(PIC_VIRTIO + 32, 1, 0x11);

    *GICD_reg(GICD_CTLR) |= 1;
    *GICC_reg(GICC_CTLR) |= 1;
}

void gic_set(int n, ISR isr)
{
    if (n < 64) {
        isrs[n] = isr;
    }
}

// ask the gic what interrupt we should serve
int
gic_handler(void)
{
    int irq;
    irq = *GICC_reg(GICC_IAR);
    if(irq == 1023)
        default_isr();
    isrs[irq]();
    *(uint*)GICC_reg(GICC_EOIR) = irq;
    return irq;
}
