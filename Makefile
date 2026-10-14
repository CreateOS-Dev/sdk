ROOTFS_MAKER := $(abspath ../rootfs_maker)
SDK_BUILD ?= $(ROOTFS_MAKER)/../build/createos-app-sdk
TOOLCHAIN_PREFIX ?= $(ROOTFS_MAKER)/../build/toolchain/x86_64-elf
PREFIX ?= /usr
DESTDIR ?=
CC := $(TOOLCHAIN_PREFIX)/bin/x86_64-elf-gcc
AR := $(TOOLCHAIN_PREFIX)/bin/x86_64-elf-ar
CFLAGS := -m64 -ffreestanding -fno-pic -fno-stack-protector -mno-red-zone \
	-Wall -Wextra -I$(ROOTFS_MAKER)/lib -Iinclude

.PHONY: all install clean

all: $(SDK_BUILD)/lib/libcreateos-uikit.a \
	$(SDK_BUILD)/lib/libcreateos-network.a \
	$(SDK_BUILD)/lib/libcreateos-http.a \
	$(SDK_BUILD)/lib/libcreateos-user.a \
	$(SDK_BUILD)/lib/libcreateos-startup.a

$(SDK_BUILD)/lib/http.o: src/http.c include/createos/http.h
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(SDK_BUILD)/lib/libcreateos-http.a: $(SDK_BUILD)/lib/http.o
	$(AR) rcs $@ $^

$(SDK_BUILD)/lib/network.o: src/network.c include/createos/network.h \
		$(ROOTFS_MAKER)/lib/user_syscall.h
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(SDK_BUILD)/lib/libcreateos-network.a: $(SDK_BUILD)/lib/network.o
	$(AR) rcs $@ $^

$(SDK_BUILD)/lib/uikit.o: src/uikit.c include/createos/uikit.h \
		$(ROOTFS_MAKER)/lib/user_syscall.h
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(SDK_BUILD)/lib/libcreateos-uikit.a: $(SDK_BUILD)/lib/uikit.o
	$(AR) rcs $@ $^

$(SDK_BUILD)/lib/syscalls.o: $(ROOTFS_MAKER)/lib/syscalls.S
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(SDK_BUILD)/lib/userlib.o: $(ROOTFS_MAKER)/lib/userlib.c \
		$(ROOTFS_MAKER)/lib/stdlib.h
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(SDK_BUILD)/lib/posix.o: $(ROOTFS_MAKER)/lib/posix.c \
		$(ROOTFS_MAKER)/lib/dirent.h \
		$(ROOTFS_MAKER)/lib/errno.h \
		$(ROOTFS_MAKER)/lib/fcntl.h \
		$(ROOTFS_MAKER)/lib/stdio.h \
		$(ROOTFS_MAKER)/lib/sys/stat.h \
		$(ROOTFS_MAKER)/lib/sys/wait.h \
		$(ROOTFS_MAKER)/lib/unistd.h \
		$(ROOTFS_MAKER)/lib/user_syscall.h
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(SDK_BUILD)/lib/libcreateos-user.a: $(SDK_BUILD)/lib/syscalls.o \
		$(SDK_BUILD)/lib/userlib.o $(SDK_BUILD)/lib/posix.o
	$(AR) rcs $@ $^

$(SDK_BUILD)/lib/command_start.o: $(ROOTFS_MAKER)/lib/command_start.S
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(SDK_BUILD)/lib/libcreateos-startup.a: $(SDK_BUILD)/lib/command_start.o
	$(AR) rcs $@ $^

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/include/createos \
		$(DESTDIR)$(PREFIX)/lib/createos
	cp include/createos/*.h $(DESTDIR)$(PREFIX)/include/createos/
	cp $(SDK_BUILD)/lib/libcreateos-uikit.a \
		$(SDK_BUILD)/lib/libcreateos-network.a \
		$(SDK_BUILD)/lib/libcreateos-http.a \
		$(SDK_BUILD)/lib/libcreateos-user.a \
		$(SDK_BUILD)/lib/libcreateos-startup.a \
		$(DESTDIR)$(PREFIX)/lib/createos/

clean:
	rm -rf $(SDK_BUILD)
