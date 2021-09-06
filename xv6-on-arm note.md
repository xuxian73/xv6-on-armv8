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

## 7.18-7.19

### progress

* spinlock: 实现spinlock，一直循环直到获得锁
* kalloc:简单的内存分配器kmem内有一个freelist，通过run实现，类似Linux list
* vm.c: mappages/walk/walkaddr/page_init
* main.c: 内存管理的初始化仍有bug。首先是把end到INIT_KERNTOP的内存交给kmem，因为这一块在start时已经装进了页表，所以可以拿来预留给kernel_pgtbl，之后page_init利用这一块区域建立页表，映射INIT_KERNTOP到PHYSTOP。kinit再利用这个映射，把剩下的页交给kmem管理（仍有bug）。之后printfinit。

## 7.20-7.22

### progress

主要在进程与调度上

* main.c：首先调用procinit，初始化nextpid锁，初始化线程池每个线程的锁，为每个线程分配一个kernel stack。之后调用userinit初始化第一个线程initcode的context。
* initcode.S：x0放SYS_exec，x1放init，x2放argv，只后用svc调用exec syscall
* proc.c：userinit函数首先调用allocproc，从线程池获得一个UNUSED的线程，为其分配pid，然后在其kstack中由上至下分配trapframe，trapret地址，kstack顶，context，然后为proc分配一个页表页。从allocproc出来后，将initcode对应的页载入页表，设置p->trapfram的lr为0，elr为0，设置为RUNNABLE。
* scheduler()：scheduler发现第一个RUNNABLE的线程也就是initcode后，载入其页表基地址，调用swtch函数，swtch函数将当前CPU（内核态）的context压入栈中，然后将栈地址存到cpu->context中，然后将sp指向proc的context，将proc的context从栈中取出，（同时也完成了kernel stack的切换）。因为lr已经设置为forkret+8，所以br x30跳转到forkret+8。在ARMv8中栈帧的结构如下。因此，编译结果在forkret开始会将x29(fp),x30(lr)压入栈，最后会将x29,x30取出。而我们的做法是跳过压入栈的指令然后在kstack中存入想要的x29与x30（trapret)之后ret就会到达trapret，trapret将栈中的trapframe取出，包括通用寄存器，以及spsr_el1, elr_el1, sp_el0,之后eret就会跳到elr_el1的位置，sp跳到sp_el0并在用户态运行。

![image-20210723111640839](/Users/xuxian/Library/Application Support/typora-user-images/image-20210723111640839.png)

## 7.23

### progress

proc.c: 

1. fork函数首先调用allocproc，然后uvmcopy父亲的pagetable， sz，trapframe并把trapframe中的x0设置为0 使得child的返回值为0，父亲返回pid

2. exit函数首先关闭所有开启的文件，然后唤醒initproc，在设置zombie之前wakeup initproc是安全的，因为即使initporc要查看当前process也会因为锁的缘故无法查看其是zombie的。

   ![image-20210727204934335](/Users/xuxian/Library/Application Support/typora-user-images/image-20210727204934335.png)

3. kill函数只将要kill的对象的kill设置为1，当改process进入或离开kernel时，会检查kill值，若为1就会exit

4. wakeup1唤醒线程p如果他sleeping在wait()中



## 8.8

### progress

file system(from bottom to up)

1. memide.c: 将blocks存在在内存之中，disksize:block的数量，memdisk为fs的开始iderw若write=1，则将memdisk+blockno*BSIZE的一块读到buf中。
2. bio.c: buffer cache层。是一个buf结构体的链表，bget得到一个locked的buffer。bread、bwrite检查若为valid则可以直接用，否则要从memide中读。一个线程使用完buf后要调用brelse，brelse检查是否refcnt变为0，若变为0就把它放在链表首，用于LRU，bpin()bunpin()增、减refcnt
3. log.c：一个log transaction可以包括多个FS system call，commit只在没有fs systemcall时执行，所以不用担心commit时有FS syscall。logheader包括log的数量以及每个log block的blockno。initlog进行初始化并从log 进行recover。begin_op()与end_op()用来决定当前FS syscall能否直接进行（考虑到log在commit，log block是否充足）,end_op()若发现没有syscall在运行，就进行commit。commit()函数首先把cache中修改的块写到log中，在修改logheader，然后将其写到disk中，最后修改log header，把log数改为0，再写log header。这样若中途断电就可以根据log的信息进行恢复。log_write找到与buf的blockno对应的log block，如果没找到，就分配一个log block。
4. fs.c：为高级的syscall提供manipulation routine。fsinit(dev)从dev读出superblock，bzero将一个block清零，balloc分配一个block



### Timer

CNTPCT_EL0: report current system count

## Git Node

7.14-7.16: boot 初步完成，提交一个commit “boot”到master

7.17： 提交commit “exception vector”

7.20:提交一个commit"memory manage"

## Reference

1. ARM启动 https://blog.csdn.net/omnispace/article/details/50747071
2. ARM registerhttps://developer.arm.com/documentation/ddi0595/2021-06/
3. ARM exception https://developer.arm.com/documentation/102412/0100/Handling-exceptions
4. arrch64 momory model https://developer.arm.com/documentation/102376/0100/Describing-the-memory-type
5. UART PL011 https://developer.arm.com/documentation/ddi0183/g/programmers-model/register-descriptions/data-register--uartdr?lang=en