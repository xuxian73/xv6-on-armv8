echo running xv6 on armV8 in debug mode

qemu-system-aarch64 -machine virt -cpu cortex-a57 \
-machine type=virt -m 128 -nographic \
-singlestep -kernel kernel.elf -s -S \