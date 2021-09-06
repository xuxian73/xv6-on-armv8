K=kernel
U=user

OBJS = \
	$K/start.o\
	$K/bio.o\
	$K/console.o\
	$K/debug.o\
	$K/exec.o\
	$K/file.o\
	$K/fs.o\
	$K/kalloc.o\
	$K/log.o\
	$K/main.o\
	$K/gic.o\
	$K/pipe.o\
	$K/printf.o\
	$K/proc.o\
	$K/sleeplock.o\
	$K/string.o\
	$K/spinlock.o\
	$K/swtch.o\
	$K/syscall.o\
	$K/sysproc.o\
	$K/sysfile.o\
	$K/timer.o\
	$K/trap_asm.o\
	$K/trap.o\
	$K/uart.o\
	$K/virtio_disk.o\
	$K/vm.o\
	$K/entry.o\
	
QEMU = qemu-system-arm
CROSSCOMPILE = aarch64-none-elf-

CC = $(CROSSCOMPILE)gcc
AS = $(CROSSCOMPILE)as
LD = $(CROSSCOMPILE)ld
OBJCOPY = $(CROSSCOMPILE)objcopy 
OBJDUMP = $(CROSSCOMPILE)objdump 

CFLAGS = -march=armv8-a -mtune=cortex-a57 -fno-pic -static -fno-builtin -fno-strict-aliasing -Wall -Werror -I. -g 
LDFLAGS = -L.
ASFLAGS = -march=armv8-a

LIBGCC = $(shell $(CC) -print-libgcc-file-name)

# host compiler
HOSTCC_preferred = gcc
define get_hostcc
    $(if $(shell which $(HOSTCC_preferred)),$(HOSTCC_preferred),"cc")
endef
HOSTCC := $(call get_hostcc)

# general rules
quiet-command = $(if $(V),$1,$(if $(2),@echo $2 && $1, @$1))

LINK_BIN = $(call quiet-command,$(LD) $(LDFLAGS) \
	-T $(1) -o $(2) $(3) $(LIBS) -b binary $(4), "  LINK     $(TARGET_DIR)$@")

LINK_INIT = $(call quiet-command,$(LD) $(LDFLAGS) \
	$(1) -o $@.out $<, "  LINK     $(TARGET_DIR)$@")
OBJCOPY_INIT = $(call quiet-command,$(OBJCOPY) \
	-S -O binary --prefix-symbols="_binary_$@" $@.out $@, "  OBJCOPY  $(TARGET_DIR)$@")

build-directory = $(shell mkdir -p build build/device build/lib build/kernel) 

UPROGS=\
	$U/_init\
	$U/_sh\
	
ULIB = $U/ulib.o $U/usys.o $U/printf.o $U/umalloc.o

_%: %.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(OBJDUMP) -S $@ > $*.asm
	$(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $*.sym

$U/usys.o: $U/usys.S
	$(CC) $(CFLAGS) -c -o $U/usys.o $U/usys.S

mkfs/mkfs: mkfs/mkfs.c $K/fs.h $K/param.h
	gcc -Werror -Wall  -I. -o mkfs/mkfs mkfs/mkfs.c

fs.img: mkfs/mkfs README $(UPROGS)
	mkfs/mkfs fs.img README $(UPROGS)

build/%.o: %.c
	$(call build-directory)
	$(call quiet-command,$(CC) $(CFLAGS) \
		-c -o $@ $<,"  CC       $(TARGET_DIR)$@")

AS_WITH = $(call quiet-command,$(CC) $(ASFLAGS) \
		$(1) -c -o $@ $<,"  AS       $(TARGET_DIR)$@")

build/%.o: %.S
	$(call build-directory)
	$(call AS_WITH, )

kernel.elf: $(addprefix build/,$(OBJS)) $K/kernel.ld build/initcode fs.img
	$(call LINK_BIN, $K/kernel.ld, kernel.elf, \
		$(addprefix build/,$(OBJS)), build/initcode, fs.img )
	$(OBJDUMP) -S kernel.elf > kernel.asm
	$(OBJDUMP) -t kernel.elf | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > kernel.sym

build/initcode.o:	$U/initcode.S
	$(call AS_WITH, -nostdinc -I.)

build/initcode: build/initcode.o
	$(call LINK_INIT, -N -e start -Ttext 0)
	$(call OBJCOPY_INIT)
	$(OBJDUMP) -S $< > initcode.asm

qemu: kernel.elf
#	@clear
#	@echo "Press Ctrl-A and then X to terminate QEMU session\n"
# $(QEMU) -M versatilepb -m 128 -cpu arm1176  -nographic -kernel kernel.elf

clean: 
	rm -rf build
	rm -f *.o *.d *.asm *.sym vectors.S bootblock entryother \
	initcode initcode.out kernel xv6.img fs.img kernel.elf memfs \
	mkfs/mkfs */*.o */*.d \
	$(UPROGS)\
	make -C tools clean
	make -C usr clean
