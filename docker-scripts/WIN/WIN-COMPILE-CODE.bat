cd ../..
docker run -it --rm -v %cd%:/ospx ospx:build ./ospx/build-all.sh
pause