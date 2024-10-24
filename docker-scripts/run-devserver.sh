#!/bin/bash
addrs=(`hostname -I`)
corepath=(`cat /proc/sys/kernel/core_pattern`)
docker run -d \
      	--mount type=bind,src=`pwd`/../build,dst=/home/game/dev \
      	--mount type=bind,src=${corepath%/*},dst=${corepath%/*} \
	--ulimit core=-1 \
	-p "${addrs[0]}:27960:27960/udp"   \
	-e "MAPS=adlernest_b3:te_escape2:te_frostbite"   \
	-e "PASSWORD=war"   \
	-e "REFEREEPASSWORD=pass123" \
	-e "SERVERCONF=comp" \
	rtcwpro/server-dev:1.0
