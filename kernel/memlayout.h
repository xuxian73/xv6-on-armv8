// Memory layout

#define KERNBASE    0xFFFFFFFF00000000
// First kernel virtual ram address
// VA: 0xffffffff40000000 ==> PA: 0x40000000 (PHY_START)

#define KERN_SZ     (uint64)0x200000
#define INIT_KERNTOP (KERN_SZ + PHYSTART)

#define V2P(va) (((uint64) (va)) - (uint64)KERNBASE)
#define P2V(pa) (((uint64) (pa)) + (uint64)KERNBASE)