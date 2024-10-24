#!/bin/bash

git clone -b sandbox https://github.com/rtcwmp-com/rtcwPro rtcwPro-sandbox
cd rtcwPro-sandbox/src
make all
cp -R /home/compile/rtcwPro-sandbox/build/* /home/compile/mnt
