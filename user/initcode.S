#include "../kernel/syscall.h"

.global start
start:
    ldr     x0, =init
    ldr     x1, =argv
    mov     x7, #SYS_exec
    svc     0x00

# for(;;) exit();
exit:
    mov     x7, #SYS_exit
    svc     0x00
    b       exit

init:
    .string "/init\0"

# char *argv[] = { init, 0 };
.p2align 2
argv:
    .long init
    .long 0