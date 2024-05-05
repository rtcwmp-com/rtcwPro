#!/bin/bash
argv=("$@")
if [ -z "${argv[0]}" ]
then
	        echo "Usage: $0 <branch name>"
		        exit 1
fi

docker pull snappas/rtcwpro-server:${argv[0]}

addrs=(`hostname -I`)
docker run -d \
	-p "${addrs[0]}:27960:27960/udp"   \
	-e "MAPS=adlernest_b3:te_escape2:te_frostbite"   \
	-e "PASSWORD=war"   \
	-e "REFEREEPASSWORD=pass123" \
	-e "SERVERCONF=comp" \
	snappas/rtcwpro-server:${argv[0]}
