#!/bin/sh

OUTPUT_FOLDER=/rtcwpro/output
mkdir -p "$OUTPUT_FOLDER"

echo '############################################################################################'
echo '##################################    BUILD SERVER    #######################################'
echo '############################################################################################'
cd /rtcwpro/src/unix
echo ''
echo ''
echo ''
make -f makefile-win
rm ../*/*.o
mv wolfded.x86 $OUTPUT_FOLDER
