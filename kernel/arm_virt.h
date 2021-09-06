//
// Board information for ARM virt
//
#ifndef __ARM_VIRT__
#define __ARM_VIRT__

/*   Copy from qemu hw/arm/virt  */
// static const MemMapEntry base_memmap[] = {
//     /* Space up to 0x8000000 is reserved for a boot ROM */
//     [VIRT_FLASH] =              {          0, 0x08000000 },
//     [VIRT_CPUPERIPHS] =         { 0x08000000, 0x00020000 },
//     /* GIC distributor and CPU interfaces sit inside the CPU peripheral space */
//     [VIRT_GIC_DIST] =           { 0x08000000, 0x00010000 },
//     [VIRT_GIC_CPU] =            { 0x08010000, 0x00010000 },
//     [VIRT_GIC_V2M] =            { 0x08020000, 0x00001000 },
//     [VIRT_GIC_HYP] =            { 0x08030000, 0x00010000 },
//     [VIRT_GIC_VCPU] =           { 0x08040000, 0x00010000 },
//     /* The space in between here is reserved for GICv3 CPU/vCPU/HYP */
//     [VIRT_GIC_ITS] =            { 0x08080000, 0x00020000 },
//     /* This redistributor space allows up to 2*64kB*123 CPUs */
//     [VIRT_GIC_REDIST] =         { 0x080A0000, 0x00F60000 },
//     [VIRT_UART] =               { 0x09000000, 0x00001000 },
//     [VIRT_RTC] =                { 0x09010000, 0x00001000 },
//     [VIRT_FW_CFG] =             { 0x09020000, 0x00000018 },
//     [VIRT_GPIO] =               { 0x09030000, 0x00001000 },
//     [VIRT_SECURE_UART] =        { 0x09040000, 0x00001000 },
//     [VIRT_SMMU] =               { 0x09050000, 0x00020000 },
//     [VIRT_PCDIMM_ACPI] =        { 0x09070000, MEMORY_HOTPLUG_IO_LEN },
//     [VIRT_ACPI_GED] =           { 0x09080000, ACPI_GED_EVT_SEL_LEN },
//     [VIRT_NVDIMM_ACPI] =        { 0x09090000, NVDIMM_ACPI_IO_LEN},
//     [VIRT_MMIO] =               { 0x0a000000, 0x00000200 },
//     /* ...repeating for a total of NUM_VIRTIO_TRANSPORTS, each of that size */
//     [VIRT_PLATFORM_BUS] =       { 0x0c000000, 0x02000000 },
//     [VIRT_SECURE_MEM] =         { 0x0e000000, 0x01000000 },
//     [VIRT_PCIE_MMIO] =          { 0x10000000, 0x2eff0000 },
//     [VIRT_PCIE_PIO] =           { 0x3eff0000, 0x00010000 },
//     [VIRT_PCIE_ECAM] =          { 0x3f000000, 0x01000000 },
//     /* Actual RAM size depends on initial RAM and device memory settings */
//     [VIRT_MEM] =                { GiB, LEGACY_RAMLIMIT_BYTES },
// };

// RAM start form 1GiB
#define PHYSTART 0x40000000
// 128MB RAM
#define PHYSTOP  (0x08000000 + PHYSTART)

// Device Base
#define GICBASE     0x08000000  // GIC distributor (interrupt controller)
#define UARTBASE    0x09000000  // Universal Asynchronous Receiver/Transmitter
#define VIRTIOBASE  0x0a000000  // Memory-mapped IO
#define DEV_SZ      0x01000000

#define UART_CLK    24000000    // UART clk rate

#define TIMER0          0x1c110000
#define TIMER1          0x1c120000
#define CLK_HZ          1000000     // the clock is 1MHZ

#define PIC_TIMER01     13      
#define PIC_TIMER23     11
#define PIC_UART0       1
#define PIC_VIRTIO      16

#endif
