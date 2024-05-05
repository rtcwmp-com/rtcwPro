#!/bin/bash
argv=("$@")

git clone -b ${argv[0]} https://github.com/rtcwmp-com/rtcwPro rtcwpro
cd rtcwpro/src
make all
