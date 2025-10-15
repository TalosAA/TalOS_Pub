#! /bin/bash
. ./prepare-env.sh
make
/bin/bash ./qemu_debug.sh