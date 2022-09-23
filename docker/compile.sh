#!/bin/sh

cd /u-boot-ap105

export SDK_DIR=/openwrt-sdk
export STAGING_DIR=${SDK_DIR}/staging_dir
export PATH=${STAGING_DIR}/toolchain-mips_24kc_gcc-8.4.0_musl/bin:${PATH}

make ap105_config

make -j 16 ARCH=mips CROSS_COMPILE=mips-openwrt-linux-musl- u-boot.bin
