#!/bin/bash

read -p "Steam username: " username
read -p "Steam password: " password
read -p "Steam guardcode (press enter to skip): " guardcode

if [[ -z "$guardcode" ]];
then
	docker build --build-arg STEAM_USER="${username}" --build-arg STEAM_PASS="${password}"  -t rtcwpro/rtcw-base:1.0 -f dockerfiles/basegame ./dockerfiles
else
	docker build --build-arg STEAM_USER="${username}" --build-arg STEAM_PASS="${password}" --build-arg GUARDCODE="${guardcode}" -t rtcwpro/rtcw-base:1.0 -f dockerfiles/basegame ./dockerfiles
fi
