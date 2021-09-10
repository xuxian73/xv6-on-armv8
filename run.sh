echo running xv6 on armV8
qemu-system-aarch64 -machine virt -cpu cortex-a57 \
-machine type=virt -m 128 -nographic \
-drive file=fs.img,if=none,format=raw,id=x0 \
-device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 \
-singlestep -kernel kernel.elf
