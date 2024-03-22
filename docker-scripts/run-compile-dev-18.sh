#!/bin/bash
if [ ! -d "../build" ]; then
	mkdir ../build
fi
docker run --mount type=bind,src=`pwd`/../build,dst=/home/compile/mnt rtcwpro/compile-dev-18:1.0
