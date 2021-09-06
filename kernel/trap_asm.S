/* 
    Only implement EL0/1.
    Therefore, the exception lower the level must from 
    el0 to el1. lower save SP_EL0, current save sp.
*/

#define verror(type) .align 7;  mov x0, #(type); bl bad_handler; b .;

.global trapret
.macro current_entry
    /* x19-x28 callee-saved register */
    /* See the trpframe struct. From bottom to top
    is the general register x0-x30(lr), than sp, elr, spsr */
    sub sp, sp, #0x20
    stp	x28, x29, [sp, #-16]!
	stp	x26, x27, [sp, #-16]!
	stp	x24, x25, [sp, #-16]!
	stp	x22, x23, [sp, #-16]!
	stp	x20, x21, [sp, #-16]!
	stp	x18, x19, [sp, #-16]!
	stp	x16, x17, [sp, #-16]!
	stp	x14, x15, [sp, #-16]!
	stp	x12, x13, [sp, #-16]!
	stp	x10, x11, [sp, #-16]!
	stp	x8, x9, [sp, #-16]!
	stp	x6, x7, [sp, #-16]!
	stp	x4, x5, [sp, #-16]!
	stp	x2, x3, [sp, #-16]!
	stp	x0, x1, [sp, #-16]!
    add x21, sp, #0x110
    mrs x22, ELR_EL1 
    mrs x23, SPSR_EL1 
    stp x30, x21, [sp, #0xf0]
    stp x22, x23, [sp, #0x100]
.endm

.macro current_ret
    ldp x22, x23, [sp, #0x100]
    ldp x30, x21, [sp, #0xf0]
    msr SPSR_EL1, x23
    msr ELR_EL1, x22
    mov x29, sp
    add sp, sp, #0x110
    ldp	x0, x1, [x29], #16
	ldp	x2, x3, [x29], #16
	ldp	x4, x5, [x29], #16
	ldp	x6, x7, [x29], #16
	ldp	x8, x9, [x29], #16
	ldp	x10, x11, [x29], #16
	ldp	x12, x13, [x29], #16
	ldp	x14, x15, [x29], #16
	ldp	x16, x17, [x29], #16
	ldp	x18, x19, [x29], #16
	ldp	x20, x21, [x29], #16
	ldp	x22, x23, [x29], #16
	ldp	x24, x25, [x29], #16
	ldp	x26, x27, [x29], #16
	ldr x28, [x29], #8
	ldr x29, [x29]
    eret
.endm

.macro lower_entry
    sub sp, sp, #0x20
    stp	x28, x29, [sp, #-16]!
	stp	x26, x27, [sp, #-16]!
	stp	x24, x25, [sp, #-16]!
	stp	x22, x23, [sp, #-16]!
	stp	x20, x21, [sp, #-16]!
	stp	x18, x19, [sp, #-16]!
	stp	x16, x17, [sp, #-16]!
	stp	x14, x15, [sp, #-16]!
	stp	x12, x13, [sp, #-16]!
	stp	x10, x11, [sp, #-16]!
	stp	x8, x9, [sp, #-16]!
	stp	x6, x7, [sp, #-16]!
	stp	x4, x5, [sp, #-16]!
	stp	x2, x3, [sp, #-16]!
	stp	x0, x1, [sp, #-16]!
    mrs x21, SP_EL0
    mrs x22, ELR_EL1
    mrs x23, SPSR_EL1
    stp x30, x21, [sp, #0xf0]
    stp x22, x23, [sp, #0x100]
.endm

.macro lower_ret
    ldp x22, x23, [sp, #0x100]
    ldp x30, x21, [sp, #0xf0]
    msr SPSR_EL1, x23
    msr ELR_EL1, x22
    msr SP_EL0, x21
    mov x29, sp
    add sp, sp, #0x110
    ldp	x0, x1, [x29], #16
	ldp	x2, x3, [x29], #16
	ldp	x4, x5, [x29], #16
	ldp	x6, x7, [x29], #16
	ldp	x8, x9, [x29], #16
	ldp	x10, x11, [x29], #16
	ldp	x12, x13, [x29], #16
	ldp	x14, x15, [x29], #16
	ldp	x16, x17, [x29], #16
	ldp	x18, x19, [x29], #16
	ldp	x20, x21, [x29], #16
	ldp	x22, x23, [x29], #16
	ldp	x24, x25, [x29], #16
	ldp	x26, x27, [x29], #16
	ldr x28, [x29], #8
	ldr x29, [x29]
    eret
.endm

trapret:
    lower_ret
/* Exception vectors */
.align 11
.global vectors
vectors:
    // Current EL, using SP_EL0
    verror(0)
    verror(1)
    verror(2)
    verror(3)

    // Current EL, using SP_ELx
    .align 7
    b   _current_sp_elx_sync
    .align 7
    b   _current_sp_elx_irq
    verror(6)
    verror(7)

    // Lower EL, using SP_EL lower
    .align 7
    b   _lower_sync
    .align 7
    b   _lower_irq
    verror(10)
    verror(11)

    .align 6
_current_sp_elx_sync:
    current_entry
    mrs x2, esr_el1     // exception syndrome register
    lsr x24, x2, #26    // get EC(exception class) seg
    cmp x24, #0x25
    b.eq    el1_da      // el1 data abort 
    cmp x24, #0x21
    b.eq    el1_ia      // el1 instruction abort
    mov x0, #4
    bl      bad_handler
    b   .

el1_da:
    mov x0, sp
    mov x1, #1
    bl dabort_handler
    b   .

el1_ia:
    mov x0, sp
    mov x1, #1
    bl iabort_handler
    b   .

_current_sp_elx_irq:
    current_entry
    mov x0, sp
    mov x1, #1
    mrs x2, esr_el1
    bl irq_handler
    current_ret

_lower_sync:
    lower_entry
    mrs x2, esr_el1
    lsr x24, x2, #26
    cmp x24, #0x24
    b.eq    el0_da
    cmp x24, #0x20
    b.eq    el0_ia
    cmp x24, #0x15
    b.eq    el0_svc
    mov x0, #8
    bl      bad_handler
    b   .

el0_da:
    mov x0, sp
    mov x1, #0
    bl dabort_handler
    b   .

el0_ia:
    mov x0, sp
    mov x1, #0
    bl iabort_handler
    b   .

el0_svc:
    mov	x0, sp
	mov	x1, #0
	bl	swi_handler
	lower_ret

_lower_irq:
    lower_entry
    mov x0, sp
    mov x1, #0
    mrs x2, esr_el1
    bl irq_handler
    lower_ret
