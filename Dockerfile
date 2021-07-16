# Dokerfile
FROM ubuntu:latest
RUN apt-get update
RUN apt-get install -y qemu-system-arm
RUN apt-get install -y gcc-arm-none-eabi
RUN apt-get install -y gdb-multiarch
RUN apt-get install -y build-essential
RUN apt-get install -y gcc-aarch64-linux-gnu
CMD ["bash"]
