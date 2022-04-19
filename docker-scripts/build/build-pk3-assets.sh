#!/bin/sh

PK3_NAME=rtcwpro_assets.pk3
OUTPUT_FOLDER=/rtcwpro/output
CONTENT_FOLDER=/rtcwpro/MAIN

if [ ! -d "$OUTPUT_FOLDER" ]; then
    echo "Output folder does not exist: $OUTPUT_FOLDER"
    exit;
fi

echo '############################################################################################'
echo '##################################    BUILD PK3 ASSETS    ##################################'
echo '############################################################################################'
rm -f $OUTPUT_FOLDER/$PK3_NAME
cd $CONTENT_FOLDER && zip -r $OUTPUT_FOLDER/$PK3_NAME ./* #&& cd -
#zip -j -r $OUTPUT_FOLDER/$PK3_NAME
# $OUTPUT_FOLDER/ui*.dll $OUTPUT_FOLDER/cgame*.dll $OUTPUT_FOLDER/ui*.so $OUTPUT_FOLDER/cgame*.so
