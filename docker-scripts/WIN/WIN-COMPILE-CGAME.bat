cd ../..
docker run -it --rm -v %cd%:/rtcwpro rtcwpro:build /rtcwpro/docker-scripts/build/build-cgame.sh && ./rtcwpro/docker-scripts/build/build-cgame.sh
pause