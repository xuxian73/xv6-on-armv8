#include "../kernel/syscall.h"

.global start
start:
    ldr     x1, =init
    ldr     x2, argv
    mov     x0, #SYS_exec
    svc     0x00

# for(;;) exit();
exit:
    mov     x0, #SYS_exit
    svc     0x00
    b       exit

init:
    .string "/init\0"

# char *argv[] = { init, 0 };
.p2align 2
argv:
    .long init
    .long 0
