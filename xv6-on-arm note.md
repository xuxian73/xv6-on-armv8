[toc]

# xv6-on-arm note

## 7.14

### progress

* kernel.ld: 用链接文件规定可执行文件框架，操作系统从0x40000000开始执行，然后跳转到start，kernel在高地址执行，但是加载在低地址。

  在bss段后，有entry使用的stack，在之后是kernel、user的pgtbl，接着是4个l1pgtbl，所以总共地址有32位

* mmu.h:接口如doc,定义了PGD、PMD、PTE相关宏，以及PTE的state信息

* entry.s:首先选择在EL1 系统模式下进行（用msr，isb）之后将sp指向ld分好的栈，之后用循环清楚bss段，最后跳到start

### doc

![image-20210714211954013](/Users/xuxian/Library/Application Support/typora-user-images/image-20210714211954013.png)

![image-20210714212012337](/Users/xuxian/Library/Application Support/typora-user-images/image-20210714212012337.png)

- Unprivileged eXecute Never (UXN) and Privileged eXecute Never (PXN) are execution permissions.
- AF is the access flag.
- SH is the shareable attribute.
- AP is the access permission.
- NS is the security bit, but only at EL3 and Secure EL1.
- Indx is the index into the MAIR_EL*n.*

![image-20210715153617193](/Users/xuxian/Library/Application Support/typora-user-images/image-20210715153617193.png)





![image-20210714222206230](/Users/xuxian/Library/Application Support/typora-user-images/image-20210714222206230.png)

![image-20210717113410382](/Users/xuxian/Library/Application Support/typora-user-images/image-20210717113410382.png)



For example, the function `foo()` can use registers `X0` to `X15` without needing to preserve their values. However, if `foo()` wants to use `X19` to `X28` it must save them to stack first, and then restore from the stack before returning.

Some registers have special significance in the PCS:

- `XR` - This is an indirect result register. If foo() returned a struct, then the memory for struct would be allocated by the caller, main() in the earlier example. `XR` is a pointer to the memory allocated by the caller for returning the struct.
- `IP0 and IP1` - These registers are intra-procedure-call corruptible registers. These registers can be corrupted between the time that the function is called and the time that it arrives at the first instruction in the function. These registers are used by linkers to insert veneers between the caller and callee. Veneers are small pieces of code. The most common example is for branch range extension. The branch instruction in A64 has a limited range. If the target is beyond that range, then the linker needs to generate a veneer to extend the range of the branch.
- `FP` - Frame pointer.
- `LR - ``X30` is the link register (`LR`) for function calls.



**LDR r, =label 会把label表示的值加载到寄存器中，而LDR r, label会把label当做地址，把label指向的地址中的值加载到寄存器中。**



## 7.15-7.16

### progress

* string.c: move from xv6-riscv
* start.c:启动流程，准备EL1下kernel user两个pagetable，其中PGD为addr[38:31]，指向第二级tabel 即pmd，PGD只有四个entry，PMD的条目设为block类型，即不再指向下一级，而是直接指向block。设置attribute要区分dev_mem和normal memory。CPU setup包括flush icache TLB 修改CPACR_EL1（3<<20)、MDSCR_EL1、MAIR_EL1、VBAR_EL1、TCR_EL1、TTBR0/1_EL1、SCTLR_EL1。之后清零bss段，把sp设置为虚地址，即加上kernbase，然后就可以跳到main函数。

到此启动基本完成，可以开始exception的部分



## 7.17

### progress

* trap_asm.S: 建立异常向量表，base align 11 entry align 7。trapfram保存通用寄存器以及sp/elr_el1/spsr_el1/lr，sp根据current level还是lower level分成两种情况。因此有两个entry。有进行实现的有current_sp_elx_irq/current_sp_elx_sync(da和ia用于debug)、lower_irq/lower_sync(da/ia/swi用于syscall)
* trap.c： 异常向量表会跳转到这个文件中的函数。da/ia用于debug，irq之后会交给picirq，swi即software interrupt用于syscall。为实现的interrupt会跳到bad_handler

### plan

由于异常处理暂时没法往下写，先开始内存管理部分。

* 首先要完成kalloc.c用于分配物理内存
* 接着完成vm.c，实现用户态（EL0）和内核态（EL1）的地址映射函数



![img](https://documentation-service.arm.com/static/6014451a4ccc190e5e681290?token=)

# 7.18-7.19

### progress

* spinlock: 实现spinlock，一直循环直到获得锁
* kalloc:简单的内存分配器kmem内有一个freelist，通过run实现，类似Linux list
* vm.c: mappages/walk/walkaddr/page_init
* main.c: 内存管理的初始化仍有bug。首先是把end到INIT_KERNTOP的内存交给kmem，因为这一块在start时已经装进了页表，所以可以拿来预留给kernel_pgtbl，之后page_init利用这一块区域建立页表，映射INIT_KERNTOP到PHYSTOP。kinit再利用这个映射，把剩下的页交给kmem管理（仍有bug）。之后printfinit。

## Git Node

7.14-7.16: boot 初步完成，提交一个commit “boot”到master

7.17： 提交commit “exception vector”

## Reference

1. ARM启动 https://blog.csdn.net/omnispace/article/details/50747071
2. ARM registerhttps://developer.arm.com/documentation/ddi0595/2021-06/
3. ARM exception https://developer.arm.com/documentation/102412/0100/Handling-exceptions
4. arrch64 momory model https://developer.arm.com/documentation/102376/0100/Describing-the-memory-type