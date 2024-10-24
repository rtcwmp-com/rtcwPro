#!/bin/bash
output=/tmp/rtcwpro-master

git clone -b master https://github.com/rtcwmp-com/rtcwPro $output
cd $output/src
make all
