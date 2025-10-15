#!/bin/sh
set -e
. ./prepare-env.sh
. ./iso.sh

qemu-system-i386 -smp 8 -m 512 -cdrom TalOS.iso -serial stdio
