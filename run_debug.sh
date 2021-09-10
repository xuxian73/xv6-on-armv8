echo running xv6 on armV8 in debug mode

qemu-system-aarch64 -machine virt,gic-version=2 -cpu cortex-a57 \
-m 128 -nographic \
-drive file=fs.img,if=none,format=raw,id=x0 \
-device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 \
-singlestep -kernel kernel.elf -s -S \
