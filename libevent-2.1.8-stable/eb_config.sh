#!/bin/bash

ARM_GCC_HOME=/opt/gcc-linaro-5.4.1

./configure CC="$ARM_GCC_HOME/bin/arm-linux-gnueabihf-gcc"  \
    CPPFLAGS="-D_XOPEN_SOURCE=500 -P" \
    LDFLAGS="-L$ARM_GCC_HOME/lib -L$ARM_GCC_HOME/arm-linux-gnueabihf/lib" \
    --host=arm-linux-gnueabihf --disable-samples
