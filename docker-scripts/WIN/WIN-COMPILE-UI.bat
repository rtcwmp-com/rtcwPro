cd ../..
docker run -it --rm -v %cd%:/rtcwpro rtcwpro:build /rtcwpro/docker-scripts/build/build-ui.sh && ./rtcwpro/docker-scripts/build/build-ui.sh
pause