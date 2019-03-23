#!/bin/bash

make clean || echo clean

rm -f config.status
./autogen.sh || echo done


./configure --enable-u8 --without-curses --host=arm-poky-linux-gnueabi

make -j 4
