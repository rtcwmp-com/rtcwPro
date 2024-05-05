#!/bin/bash

argv=("$@")
if [ -z "${argv[0]}" ]
then
	echo "Usage: $0 <docker git image: snappas/rtcwpro> <branch name>"
	exit 1
fi

docker build --build-arg IMAGE="${argv[0]}:${argv[1]}" -t snappas/rtcwpro-server:${argv[1]} -f dockerfiles/rtcwpro-server-git ./dockerfiles
