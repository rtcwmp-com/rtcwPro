cd ../..
docker run -it --rm -v %cd%:/rtcwpro rtcwpro:build /rtcwpro/docker-scripts/build/build-pk3.sh && ./rtcwpro/docker-scripts/build/build-pk3.sh
pause