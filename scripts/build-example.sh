#!/bin/sh
set -eu

if [ "$#" -ne 1 ]; then
    printf 'Usage: %s EXAMPLE_NAME\n' "$0" >&2
    exit 2
fi

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
sdk_root=$(CDPATH= cd -- "$script_dir/.." && pwd)
example_dir="$sdk_root/examples/$1"

if [ ! -f "$example_dir/Makefile" ]; then
    printf 'Unknown CreateOS SDK example: %s\n' "$1" >&2
    exit 2
fi

toolchain_prefix=${TOOLCHAIN_PREFIX:-"$sdk_root/../build/toolchain/x86_64-elf"}
make -C "$example_dir" all \
    SDK_ROOT="$sdk_root" \
    TOOLCHAIN_PREFIX="$toolchain_prefix"
