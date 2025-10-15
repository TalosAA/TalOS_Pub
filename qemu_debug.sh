
qemu-system-i386 -d int,cpu_reset,strace,guest_errors -D ./qemu-debug-log -s -S -smp 8 -m 512 -cdrom TalOS.iso -serial stdio
