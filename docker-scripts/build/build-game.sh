#!/bin/sh

OUTPUT_FOLDER=/rtcwpro/output
mkdir -p "$OUTPUT_FOLDER"

echo '############################################################################################'
echo '##################################    BUILD GAME    ########################################'
echo '############################################################################################'
cd /rtcwpro/src/game
echo ''
echo ''
echo ''
echo '############################################################################################'
echo '##################################    LINUX GAME    ########################################'
echo '############################################################################################'
make -f makefile
rm ../*/*.o
mv qagame.mp.i386.so $OUTPUT_FOLDER
#echo ''
#echo ''
#echo ''
#echo '############################################################################################'
#echo '##################################    WIN32 GAME    ########################################'
#echo '############################################################################################'
#make -f makefile.w32
#rm ../*/*.o
#mv qagame_mp_x86.dll $OUTPUT_FOLDER
