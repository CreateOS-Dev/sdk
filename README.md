# CreateOS application SDK

The SDK builds native x86-64 `.APPL` programs for CreateOS. It provides the
startup code and syscall library, plus small UI, networking, and HTTP
helpers. The UI library draws in the text console; it is not a window system.

Use this SDK from the source checkout that matches the kernel you are
targeting. Its syscall interface and headers change along with the OS.

## Build an example

Build the cross-toolchain from the repository root, then try either example:

```sh
make toolchain
./sdk/scripts/build-example.sh hello
./sdk/scripts/build-example.sh uikit-demo
```

Build the static libraries directly with `make -C sdk`. The SDK's `install`
target stages the public headers and libraries under `$(DESTDIR)/usr/` by
default; pass `PREFIX` to choose another install prefix:

```sh
make -C sdk install DESTDIR="$PWD/build/rootfs"
```

The userland image build calls this target with its staging directory. Avoid
running `make -C sdk install` without a `DESTDIR` unless installing to the
host's configured `PREFIX` is intentional.

The generated `.APPL` is a static ELF executable. Build output stays under
`build/`, not alongside the source.

## Start an app project

Create a project Makefile that names the app and points to its sources:

```make
APP_NAME := hello
APP_VERSION := 0.1.0
APP_SRC_DIR := $(CURDIR)/src
SDK_ROOT ?= /path/to/CreateOS/sdk

include $(SDK_ROOT)/app.mk
```

Place C (`.c`) or x86-64 assembly (`.S`) files below `src/`. The build rules
find them recursively. For older one-file projects, set `APP_SRC` instead.

For a toolchain outside its default location, pass `TOOLCHAIN_PREFIX` to
Make:

```sh
make -C /path/to/hello \
  SDK_ROOT="$PWD/sdk" \
  TOOLCHAIN_PREFIX="$PWD/build/toolchain/x86_64-elf"
```

## Put an app in the guest

Copy the finished `.APPL` into `rootfs_maker/root/appcustom/`, then run
`make userapps` and rebuild the disk image. You can also use the project's
`make install` target to copy into that folder. In the guest, Explorer can
launch custom applications from `/app/custom/`.

An `.APPL` is native code, not a sandbox. Only install programs you trust.
For a complete walkthrough, see `docs/sdk/app-development.md` in the complete
CreateOS source tree.
The C programming guide is `docs/sdk/c.md`.
The same documentation tree contains the UI Kit reference
(`docs/sdk/uikit.md`), the Lua guide (`docs/sdk/lua.md`), and the x86-64
assembly guide (`docs/sdk/assembly.md`).

## What is included

- `include/createos/uikit.h`: colored text, keyboard input, mouse position,
  and full-screen console helpers.
- `include/createos/network.h`: the current limited TCP listener API.
- `include/createos/http.h`: HTTP parsing helpers.
- `src/`: implementations linked into SDK libraries.
- `examples/`: small projects that show the Makefile setup.

This SDK currently builds C and x86-64 assembly. It does not provide C++,
Rust, Go, or other guest toolchains, dynamic linking, or a complete POSIX
runtime.
