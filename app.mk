SDK_FILE_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
SDK_ROOT ?= $(SDK_FILE_DIR)
ifeq ($(wildcard $(SDK_ROOT)/../rootfs_maker/Makefile),)
ROOTFS_MAKER ?= $(SDK_ROOT)
BUILD_DIR ?= $(abspath $(CURDIR)/build/createos/$(APP_NAME))
else
ROOTFS_MAKER ?= $(abspath $(SDK_ROOT)/../rootfs_maker)
BUILD_DIR ?= $(abspath $(ROOTFS_MAKER)/../build/createos-app/$(APP_NAME))
endif
TOOLCHAIN_PREFIX ?= $(ROOTFS_MAKER)/../build/toolchain/x86_64-elf
CC := $(TOOLCHAIN_PREFIX)/bin/x86_64-elf-gcc
LD := $(TOOLCHAIN_PREFIX)/bin/x86_64-elf-ld
AR := $(TOOLCHAIN_PREFIX)/bin/x86_64-elf-ar
APP_DIR ?= .
APP_NAME ?= $(notdir $(abspath $(APP_DIR)))
APP_VERSION ?= 0.1.0
APP_CONFIG ?= $(APP_DIR)/config.cfg
APP_SRC_DIR ?= $(APP_DIR)/src
APP_SRC ?= $(APP_DIR)/main.c
APP_OBJECTS ?=
APP_CFLAGS ?=
APP_LDFLAGS ?=
APP_MAIN_DEPS ?=
APP_CLEAN_EXTRA ?=
APP_OUT := $(BUILD_DIR)/app/$(APP_NAME).APPL
APP_BUILD := $(BUILD_DIR)/app/$(APP_NAME)
APP_CONFIG_DEP := $(wildcard $(APP_CONFIG))
SDK_BUILD ?= $(BUILD_DIR)/sdk
ifeq ($(wildcard $(SDK_ROOT)/Makefile),)
SDK_LIB_DIR := $(SDK_ROOT)/lib
else
SDK_LIB_DIR := $(SDK_BUILD)/lib
endif
SDK_LIBS := $(SDK_LIB_DIR)/libcreateos-uikit.a \
	$(SDK_LIB_DIR)/libcreateos-network.a \
	$(SDK_LIB_DIR)/libcreateos-http.a \
	$(SDK_LIB_DIR)/libcreateos-user.a \
	$(SDK_LIB_DIR)/libcreateos-startup.a
ifeq ($(wildcard $(ROOTFS_MAKER)/lib/user_syscall.h),)
USER_API_INCLUDE := $(SDK_ROOT)/rootfs_maker/lib
else
USER_API_INCLUDE := $(ROOTFS_MAKER)/lib
endif
CFLAGS := -m64 -ffreestanding -fno-pic -fno-stack-protector -mno-red-zone \
	-I$(USER_API_INCLUDE) -I$(SDK_ROOT)/include -Wall -Wextra
APP_CFLAGS += -DCREATEOS_APP_NAME=\"$(APP_NAME)\" \
	-DCREATEOS_APP_VERSION=\"$(APP_VERSION)\"
ifeq ($(wildcard $(SDK_ROOT)/app.ld),)
APP_LD := $(ROOTFS_MAKER)/app.ld
else
APP_LD := $(SDK_ROOT)/app.ld
endif
CUSTOM_APP_DIR ?= $(ROOTFS_MAKER)/root/appcustom
LDFLAGS := -n -z noexecstack -T $(APP_LD)
LIBGCC := $(shell $(CC) -print-libgcc-file-name)

APP_DISCOVERED_SOURCES := $(shell find "$(APP_SRC_DIR)" -type f -print 2>/dev/null | \
	grep -E '\.(c|S)$$' | LC_ALL=C sort)
APP_DISCOVERED_C_SOURCES := $(filter %.c,$(APP_DISCOVERED_SOURCES))
APP_DISCOVERED_ASM_SOURCES := $(filter %.S,$(APP_DISCOVERED_SOURCES))

ifneq ($(strip $(APP_DISCOVERED_SOURCES)),)
APP_AUTO_OBJECTS := \
	$(patsubst $(APP_SRC_DIR)/%.c,$(APP_BUILD)/src/%.o,$(APP_DISCOVERED_C_SOURCES)) \
	$(patsubst $(APP_SRC_DIR)/%.S,$(APP_BUILD)/src/%.o,$(APP_DISCOVERED_ASM_SOURCES))
else
APP_AUTO_OBJECTS := $(APP_BUILD)/main.o
endif

.PHONY: all clean install

all: $(APP_OUT)

ifeq ($(wildcard $(SDK_ROOT)/Makefile),$(SDK_ROOT)/Makefile)
$(SDK_BUILD)/.sdk-built: $(SDK_ROOT)/Makefile $(SDK_ROOT)/src/uikit.c \
		$(SDK_ROOT)/src/network.c \
		$(SDK_ROOT)/src/http.c \
		$(SDK_ROOT)/include/createos/network.h \
		$(SDK_ROOT)/include/createos/http.h \
		$(ROOTFS_MAKER)/lib/user_syscall.h \
		$(ROOTFS_MAKER)/lib/posix.c \
		$(ROOTFS_MAKER)/lib/dirent.h \
		$(ROOTFS_MAKER)/lib/errno.h \
		$(ROOTFS_MAKER)/lib/fcntl.h \
		$(ROOTFS_MAKER)/lib/stdio.h \
		$(ROOTFS_MAKER)/lib/sys/stat.h \
		$(ROOTFS_MAKER)/lib/sys/wait.h \
		$(ROOTFS_MAKER)/lib/unistd.h
	$(MAKE) -C $(SDK_ROOT) all SDK_BUILD=$(SDK_BUILD) \
		TOOLCHAIN_PREFIX=$(TOOLCHAIN_PREFIX)
	touch $@

$(SDK_LIBS): $(SDK_BUILD)/.sdk-built
	@test -s "$@" || { \
		printf 'CreateOS SDK library was not built: %s\n' "$@" >&2; \
		exit 1; \
	}
else
$(SDK_LIBS):
	@test -s "$@" || { \
		printf 'Required CreateOS SDK library is missing: %s\n' "$@" >&2; \
		exit 1; \
	}
endif

$(APP_BUILD)/main.o: $(APP_SRC) $(APP_CONFIG_DEP) \
		$(SDK_ROOT)/include/createos/uikit.h \
		$(SDK_ROOT)/include/createos/network.h \
		$(SDK_ROOT)/include/createos/http.h \
		$(APP_MAIN_DEPS)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(APP_CFLAGS) -c $< -o $@

$(APP_BUILD)/src/%.o: $(APP_SRC_DIR)/%.c $(APP_CONFIG_DEP) \
		$(SDK_ROOT)/include/createos/uikit.h \
		$(SDK_ROOT)/include/createos/network.h \
		$(SDK_ROOT)/include/createos/http.h \
		$(APP_MAIN_DEPS)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(APP_CFLAGS) -c $< -o $@

$(APP_BUILD)/src/%.o: $(APP_SRC_DIR)/%.S $(APP_CONFIG_DEP)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(APP_OUT): $(APP_AUTO_OBJECTS) $(APP_OBJECTS) $(SDK_LIBS) $(APP_LD)
	mkdir -p $(@D)
	$(LD) $(LDFLAGS) $(APP_LDFLAGS) -e user_command_start -o $@ \
		$(APP_AUTO_OBJECTS) -u user_command_start \
		$(APP_OBJECTS) \
		--whole-archive $(SDK_LIBS) --no-whole-archive \
		--start-group $(LIBGCC) --end-group
	@printf 'Built %s v%s: %s\n' "$(APP_NAME)" "$(APP_VERSION)" "$(APP_OUT)"

clean:
	rm -rf $(APP_BUILD) $(APP_OUT) $(APP_CLEAN_EXTRA)

install: $(APP_OUT)
	mkdir -p $(CUSTOM_APP_DIR)
	cp $(APP_OUT) $(CUSTOM_APP_DIR)/$(APP_NAME).APPL
