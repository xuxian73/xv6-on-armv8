// Definition for ARM mmu
#ifndef MMU_ARM
#define MMU_ARM

// pte[1:0] indicate valid/fault/type of the entry
#define ENTRY_VALID 0x01
#define ENTRY_BLOCK 0x00
#define ENTRY_PAGE  0x02
#define ENTRY_TABLE 0x02
#define ENTRY_MASK  0x03
#define ENTRY_FAULT 0x00

//idx is the index into the MAIR_ELn
#define MEM_ATTR_IDX(x) ((uint64)x << 2)

//non-secure
#define ENTRY_NS    (1 << 5)

//Access Permission
#define AP_0            (0 << 6)    /* EL0: N ;EL1: RW*/
#define AP_1            (1 << 6)    /* EL0: RW ;EL1: RW*/
#define AP_2            (2 << 6)    /* EL0: N ;EL1: RO*/
#define AP_3            (3 << 6)    /* EL0: RO ;EL1: RO*/
#define AP_MASK         (3 << 6)    

//Shareable attributes
#define SH_NON_SH   (0 << 8)
#define SH_UNPRED   (1 << 8)
#define SH_OUT_SH   (2 << 8)
#define SH_IN_SH    (3 << 8)

//Access Flag = 1: accessible
#define ACCESS_FLAG (1 << 10)

//User Execute Never
#define UXN ((uint64)1 << 54)
//Privileged Execute Never
#define PXN ((uint64)1 << 53)

/* 4-level page table */
// level2 PGD: entry[38:30]
#define PGD_SHIFT       30
#define PGD_SZ          (1 << PGD_SHIFT)
#define PGD_MASK        (~(PGD_SZ - 1))
#define PTRS_PER_PGD    4
// given va get the index in PGD
#define PGD_IDX(va)      (((uint64)(va) >> PGD_SHIFT) & (PTRS_PER_PGD - 1))

// level3 PMD: entry[29:21]
#define PMD_SHIFT       21
#define PMD_SZ          (1 << PMD_SHIFT)
#define PMD_MASK        (~(PMD_SZ - 1))
#define PTRS_PER_PMD    (1 << 9)
#define PMD_IDX(va)     (((uint64)(va) >> PMD_SHIFT) & (PTRS_PER_PMD - 1))

// level4 PTE: entry[20:12]
#define PTE_SHIFT       12
#define PTE_SZ          (1 << PTE_SHIFT)
#define PTE_MASK        (~(PTE_SZ - 1))
#define PTRS_PER_PTE    (1 << 9)
#define PTE_IDX(va)     (((uint64)(va) >> PTE_SHIFT) & (PTRS_PER_PTE - 1))

#define PG_ADDR_MASK 0xFFFFFFFFF000
#define PGSIZE 4096 // bytes per page
#define PGSHIFT 12  // bits of offset within a page

#define PGROUNDUP(sz)  (((sz)+PGSIZE-1) & ~(PGSIZE-1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE-1))


#endif