#!/bin/sh

PK3_NAME=rtcwpro_bin.post125.pk3
OUTPUT_FOLDER=/rtcwpro/output
CONTENT_FOLDER=/rtcwpro/MAIN

if [ ! -d "$OUTPUT_FOLDER" ]; then
    echo "Output folder does not exist: $OUTPUT_FOLDER"
    exit;
fi

echo '############################################################################################'
echo '##################################    BUILD PK3 BIN    #####################################'
echo '############################################################################################'
rm -f $OUTPUT_FOLDER/$PK3_NAME
cd $CONTENT_FOLDER ./* && cd - 
zip -j -r $OUTPUT_FOLDER/$PK3_NAME $OUTPUT_FOLDER/ui*.dll $OUTPUT_FOLDER/cgame*.dll $OUTPUT_FOLDER/qagame*.dll $OUTPUT_FOLDER/ui*.so $OUTPUT_FOLDER/cgame*.so $OUTPUT_FOLDER/qagame*.so
