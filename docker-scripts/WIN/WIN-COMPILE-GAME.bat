cd ../..
docker run -it --rm -v %cd%:/rtcwpro rtcwpro:build /rtcwpro/docker-scripts/build/build-game.sh && ./rtcwpro/docker-scripts/build/build-game.sh
pause