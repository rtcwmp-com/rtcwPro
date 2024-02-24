#!/bin/bash
addrs=(`hostname -I`)
docker run --mount type=bind,src=`pwd`../build,dst=/home/game/dev   -p "${addrs[0]}:27960:27960/udp"   -e "MAPS=adlernest_b3:te_escape2:te_frostbite"   -e "PASSWORD=war"   -e "REFEREEPASSWORD=pass123" -e "SERVERCONF=comp" rtcwpro/server-dev:1.0
