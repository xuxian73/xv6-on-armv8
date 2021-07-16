#include "memlayout.h"

.text
.align 16
.global _entry
_entry:
    # initialize sp for supervisor(EL1) mode
    mov     x0, #1   
    msr     spsel, x0   //select EL1
    isb                 //ensure inst before done

    adrp    x0, entry_stack //get 4k aligned entry stack address
    mov     sp, x0
    
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