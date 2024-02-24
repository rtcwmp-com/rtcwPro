#!/bin/bash

git clone -b feature/dockerbuild https://github.com/rtcwmp-com/rtcwPro rtcwPro-develop
cd rtcwPro-develop/src
make all
cp -R /home/compile/rtcwPro-develop/build /home/compile/build
