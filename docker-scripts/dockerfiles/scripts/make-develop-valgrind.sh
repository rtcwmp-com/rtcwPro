#!/bin/bash

git clone -b develop https://github.com/rtcwmp-com/rtcwPro rtcwPro-develop
cd rtcwPro-develop/src
make all-valgrind
cp -R /home/compile/rtcwPro-develop/build/* /home/compile/mnt
