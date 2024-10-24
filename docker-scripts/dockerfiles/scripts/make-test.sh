#!/bin/bash
output=/tmp/rtcwpro-test

git clone -b test https://github.com/rtcwmp-com/rtcwPro $output
cd $output/src
make all
