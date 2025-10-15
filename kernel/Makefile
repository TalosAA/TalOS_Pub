
CFLAGS?=-O0
CPPFLAGS?=
LDFLAGS?=
LIBS?=
GNU_STD?=-std=gnu11
BUILD_MODE?=DEBUG

BOOTDIR?=boot
INCLUDEDIR_KER?=include
INCLUDEDIR?=include
INCLUDEDIR_ARCH?=$(INCLUDEDIR_KER)/arch
INCLUDEDIR_LIBK?=$(INCLUDEDIR_KER)/libk

#Objects and executables directory tree
KERNEL_OUTDIR?=$(BUILD_MODE)
KERNEL_OUTDIR_OBJ?=$(KERNEL_OUTDIR)/obj
KERNEL_EXE?=$(KERNEL_OUTDIR)/TalOS.kernel

#Source code directory tree of the kernel
KERNELDIR=$(shell pwd)
ARCHDIR=$(KERNELDIR)/src/arch/$(HOSTARCH)
CORESYSDIR=$(KERNELDIR)/src/coresys
DEVSDIR=$(KERNELDIR)/src/devs
LIBKDIR=$(KERNELDIR)/src/libk

#Include directories
INCLUDEDIRS=\
-I$(INCLUDEDIR_KER) \
-I$(INCLUDEDIR_ARCH) \
-I$(INCLUDEDIR_LIBK)

#Generic kernel flags, they can be changed inside each make.config.
CFLAGS:=$(CFLAGS)
CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS) -D__is_kernel $(INCLUDEDIRS)
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lgcc

#The following includes define the rules to make
#the file for each folder.
include $(ARCHDIR)/make.config
include $(DEVSDIR)/make.config
include $(CORESYSDIR)/make.config
include $(LIBKDIR)/make.config

#Build Kernel's objects list
OBJS=\
$(CORESYS_OBJS) \
$(KERNEL_ARCH_OBJS) \
$(DEVS_OBJS) \
$(LIBK_OBJS)

#Build link list
LINK_LIST=\
$(OBJS) \
$(LIBS)

.PHONY: all clean install install-headers install-kernel
.SUFFIXES: .o .c .S

all: $(KERNEL_EXE)

kernel_outdir:
	$(MKDIR) -p $(KERNEL_OUTDIR)

$(KERNEL_EXE): $(OBJS) kernel_outdir
	$(CC) -xc -E $(ARCHDIR)/linker.ld.in $(INCLUDEDIRS) | grep -v '^#' > $(KERNEL_OUTDIR)/linker.ld
	$(CC) -Wl,-Map=$(KERNEL_OUTDIR)/kernel.map -T $(KERNEL_OUTDIR)/linker.ld -o $@ $(CFLAGS) $(LDFLAGS) $(LINK_LIST) -mno-red-zone
	grub-file --is-x86-multiboot $(KERNEL_EXE)

clean:
	rm -fr $(BUILD_MODE)

install: install-kernel

install-kernel: $(KERNEL_EXE)
	mkdir -p $(KERNEL_OUTDIR)/$(BOOTDIR)
	cp $(KERNEL_EXE) $(KERNEL_OUTDIR)/$(BOOTDIR)

#Include *.d files (include dependencies)
-include $(OBJS:.o=.d)
