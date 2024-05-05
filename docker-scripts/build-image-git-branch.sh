#!/bin/bash

argv=("$@")
if [ -z "${argv[0]}" ]
then
	echo "Usage: $0 <branch name>"
	exit 1
fi

docker build --build-arg BRANCH="${argv[0]}" -t snappas/rtcwpro:${argv[0]} -f dockerfiles/rtcwpro-compile ./dockerfiles
