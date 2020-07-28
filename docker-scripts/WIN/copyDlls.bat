copy "J:\GitHub\ospx-elite\output\qagame_mp_x86.dll" "J:\GitHub\Rtcw-Pro\src\rtcwpro\" /Y
copy "J:\GitHub\ospx-elite\output\cgame_mp_x86.dll" "J:\GitHub\ospx-elite\src\rtcwpro\" /Y
copy "J:\GitHub\ospx-elite\output\ui_mp_x86.dll" "J:\GitHub\ospx-elite\src\rtcwpro\" /Y
copy "J:\GitHub\ospx-elite\output\qagame_mp_x86.dll" "C:\Program Files (x86)\Return to Castle Wolfenstein\rtcwpro" /Y
copy "J:\GitHub\ospx-elite\output\cgame_mp_x86.dll" "C:\Program Files (x86)\Return to Castle Wolfenstein\rtcwpro" /Y
copy "J:\GitHub\ospx-elite\output\ui_mp_x86.dll" "C:\Program Files (x86)\Return to Castle Wolfenstein\rtcwpro" /Y

copy "J:\GitHub\ospx-elite\output\qagame_mp_x86.dll" "C:\Program Files (x86)\Return to Castle Wolfenstein Copy\rtcwpro" /Y
copy "J:\GitHub\ospx-elite\output\cgame_mp_x86.dll" "C:\Program Files (x86)\Return to Castle Wolfenstein Copy\rtcwpro" /Y
copy "J:\GitHub\ospx-elite\output\ui_mp_x86.dll" "C:\Program Files (x86)\Return to Castle Wolfenstein Copy\rtcwpro" /Y

cd ../..
docker run -it --rm -v %cd%:/rtcwpro rtcwpro:build dos2unix /rtcwpro/docker-scripts/build/build-pk3.sh 
docker run -it --rm -v %cd%:/rtcwpro rtcwpro:build ./rtcwpro/docker-scripts/build/build-pk3.sh

copy "J:\GitHub\ospx-elite\output\qagame_mp_x86.dll" "J:\GitHub\ospx-elite\src\rtcwpro" /Y
copy "J:\GitHub\ospx-elite\output\cgame_mp_x86.dll" "J:\GitHub\ospx-elite\src\rtcwpro" /Y
copy "J:\GitHub\ospx-elite\output\ui_mp_x86.dll" "J:\GitHub\ospx-elite\src\rtcwpro" /Y
copy "J:\GitHub\ospx-elite\output\rtcwpro.pk3" "J:\GitHub\ospx-elite\src\rtcwpro" /Y
copy "J:\GitHub\ospx-elite\output\rtcwpro.pk3" "C:\Program Files (x86)\Return to Castle Wolfenstein\rtcwpro" /Y
copy "J:\GitHub\ospx-elite\output\rtcwpro.pk3" "C:\Program Files (x86)\Return to Castle Wolfenstein Copy\rtcwpro" /Y
pause
