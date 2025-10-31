set -e
. ./prepare-env.sh
. ./iso.sh
qemu-system-i386 -d int,cpu_reset,strace,guest_errors -D ./qemu-debug-log -s -S -smp 8 -m 512 -cdrom TalOS.iso -serial stdio -machine q35,accel=tcg,acpi=on -monitor telnet:127.0.0.1:4444,server,nowait -device pcie-pci-bridge

