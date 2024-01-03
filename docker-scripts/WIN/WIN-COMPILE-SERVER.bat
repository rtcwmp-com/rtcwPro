cd ../..
docker run -it --rm -v %cd%:/rtcwpro rtcwpro:build dos2unix /rtcwpro/docker-scripts/build/build-server.sh 
docker run -it --rm -v %cd%:/rtcwpro rtcwpro:build ./rtcwpro/docker-scripts/build/build-server.sh
pause