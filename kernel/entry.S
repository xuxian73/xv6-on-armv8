#include "memlayout.h"

.text
.align 16
.global _entry
_entry:
    mrs     x0, mpidr_el1
    and     x0, x0, #3
    cmp     x0, #0x0
    b.ne    _open_core

    ldr x0, =0xc4000003
    ldr x1, =1         /* target CPU's MPIDR affinity */
    ldr x2, =0x40010000   /* entry point */
    ldr x3, =1         /* context ID: put into target CPU's x0 */
    hvc #0
_open_core:
    # initialize sp for supervisor(EL1) mode

    mov     x0, #1   
    msr     spsel, x0   //select EL1
    isb                 //ensure inst before done

    mrs     x1, mpidr_el1
    and     x1, x1, #3
    mov     x2, #0x1000
    adrp    x0, entry_stack //get 4k aligned entry stack address
    mul     x2, x1, x2
    sub     x0, x0, x2
    mov     sp, x0
    cmp     x1, #0
    b.ne    _done
    # clear bss section and stack
    ldr     x1, =edata_entry
    ldr     x2, =end_entry
    mov     x3, #0
_loop: 
    CMP     x1, x2
    B.GT    _done
    STR     x3, [x1]
    ADD     x1, x1, #8
    B.LT    _loop
_done:
    BL      start
    B       .           //loop forever

.global jump_stack
jump_stack: 
    mov x0, sp
    ldr x1, =KERNBASE
    add sp, x0, x1
    ret




    