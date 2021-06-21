cd ../..
docker run -it --rm -v %cd%:/rtcwpro rtcwpro:build dos2unix /rtcwpro/docker-scripts/build/build-pk3-newclient.sh 
docker run -it --rm -v %cd%:/rtcwpro rtcwpro:build ./rtcwpro/docker-scripts/build/build-pk3-newclient.sh

copy "J:\GitHub\RTCW-Pro\src\Builds\Debug\cgame\cgame_mp_x86.dll" "C:\Program Files (x86)\Return to Castle Wolfenstein Copy\rtcwpro" /Y
copy "J:\GitHub\RTCW-Pro\src\Builds\Debug\qagame\qagame_mp_x86.dll" "C:\Program Files (x86)\Return to Castle Wolfenstein Copy\rtcwpro" /Y
copy "J:\GitHub\RTCW-Pro\src\Builds\Debug\ui\ui_mp_x86.dll" "C:\Program Files (x86)\Return to Castle Wolfenstein Copy\rtcwpro" /Y

copy "J:\GitHub\RTCW-Pro\output\rtcwpro.pk3" "C:\Program Files (x86)\Return to Castle Wolfenstein Copy\rtcwpro" /Y
pause
