#!/bin/bash

argv=("$@")
if [ -z "${argv[0]}" ]
then
	echo "Usage: $0 <docker git image: snappas/rtcwpro> <branch name>"
	exit 1
fi

RTCWPRO_SRC=$(dirname `pwd`)


# build the files from github branch (these are baked into the image)
docker build \
--build-arg BRANCH="${argv[1]}" \
-t ${argv[0]}:${argv[1]} \
-f dockerfiles/rtcwpro-compile ./dockerfiles

# build the files from the local volume inside the container
#  artifacts go to <repo>/build/
docker run -it \
-v $RTCWPRO_SRC:/home/compile/code \
--workdir /home/compile/code/src \
${argv[0]}:${argv[1]} \
make all

rm -rf $RTCWPRO_SRC/build/objs

mv $RTCWPRO_SRC/build/rtcwpro/rtcwpro_bin.pk3 $RTCWPRO_SRC/build/rtcwpro/rtcwpro_bin-$(date +%Y%m%d).pk3
mv $RTCWPRO_SRC/build/rtcwpro/rtcwpro_assets.pk3 $RTCWPRO_SRC/build/rtcwpro/rtcwpro_assets-$(date +%Y%m%d).pk3
mv $RTCWPRO_SRC/build/rtcwpro/rtcwpro_models.pk3 $RTCWPRO_SRC/build/rtcwpro/rtcwpro_models-$(date +%Y%m%d).pk3

# load a shell into the image
#docker run -it -v /var/run/docker.sock:/var/run/docker.sock \
#-v $(dirname `pwd`):/home/compile/code \
#--workdir /home/compile/code/src \
#snappas/rtcwpro:develop /bin/bash


bash build-image-git-server.sh ${argv[0]} ${argv[1]}