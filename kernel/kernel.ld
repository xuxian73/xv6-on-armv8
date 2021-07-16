OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_entry)

ENTRY_INIT_STACK_SIZE = 0x2000;

SECTIONS
{
  /* the entry point, before enabling paging. The code to enable paging
   needs to have the same virtual/physical address. entry.S and start.c
   run in this initial setting.*/
  . = 0x40010000;
  .start_sec : {
    build/kernel/entry.o(.text)
    build/kernel/start.o(.text .text.*)

    build/kernel/entry.o(.rodata .rodata.*)
    build/kernel/start.o(.rodata .rodata.*)

    build/kernel/entry.o(.data .data.*)
    build/kernel/start.o(.data .data.*)

    PROVIDE(edata_entry = .);

    build/kernel/entry.o(.bss .bss.* COMMON)
    build/kernel/start.o(.bss .bss.* COMMON)

    /*define a stack for the entry*/
    . = ALIGN(0x1000);
    . += ENTRY_INIT_STACK_SIZE;

    PROVIDE (entry_stack = .);

    /* define the kernel page table, must be 4K and 4K-aligned*/
    . = ALIGN(0x1000);
    PROVIDE (_kernel_pgtbl = .);
    . += 0x1000;

    /* we also need a user page table*/
    PROVIDE (_user_pgtbl = .);
    . += 0x1000;

    /* level 2 pages 4k * 4, PGD has four entries*/
    PROVIDE (_K_l2_pgtbl = .);
    . += 0x4000;
    PROVIDE (_U_l2_pgtbl = .);
    . += 0x4000;

    PROVIDE(end_entry = .);
  }

  /*the kernel executes at the higher address space, but loaded
   at the lower memory (0x30000)*/
  . = 0xFFFFFFFF40030000;  /* HCLIN: below text symbols is in VA space **/

  .text : AT(0x40030000){ /** here to make code also copied into phymem **/
    *(.text .text.* .gnu.linkonce.t.*)
  }

  PROVIDE(etext = .);	/* Define the 'etext' symbol to this value */

  .rodata : {
    *(.rodata .rodata.* .gnu.linkonce.r.*)
  }

  /* aligned the data to a (4K) page, so it can be assigned
   different protection than the code*/
  . = ALIGN(0x1000);

  PROVIDE (data_start = .);

  .data : {
    *(.data .data.*)
  }

  PROVIDE (edata = .);

  .bss : {
    *(.bss .bss.* COMMON)
  }

  . = ALIGN(0x1000);
  PROVIDE (end = .);
}
