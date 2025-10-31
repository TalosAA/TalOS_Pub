#!/bin/sh
set -e
. ./build.sh $1

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

BUILD_TYPE=

if [ -z "$1" ]; then
	BUILD_TYPE="DEBUG"
else
	BUILD_TYPE="$1"
fi

if [ "$BUILD_TYPE" = "RELEASE" ]
then
	cp kernel/RELEASE/boot/TalOS.kernel isodir/boot/TalOS.kernel
else
	cp kernel/DEBUG/boot/TalOS.kernel isodir/boot/TalOS.kernel
fi

cp ./initrd.img isodir/boot/initrd.img

cat > isodir/boot/grub/grub.cfg << EOF
set timeout=0
menuentry "TalOS 0.0.1" {
	multiboot /boot/TalOS.kernel
	module  /boot/initrd.img
}
EOF
grub-mkrescue -o TalOS.iso isodir
