#!/bin/sh
#move to src dir
#cd /rtcwpro/src
dos2unix /rtcwpro/docker-scripts/build/build-game.sh && ./rtcwpro/docker-scripts/build/build-game.sh
dos2unix /rtcwpro/docker-scripts/build/build-cgame.sh && ./rtcwpro/docker-scripts/build/build-cgame.sh
dos2unix /rtcwpro/docker-scripts/build/build-ui.sh && ./rtcwpro/docker-scripts/build/build-ui.sh
dos2unix /rtcwpro/docker-scripts/build/build-pk3-assets.sh && ./rtcwpro/docker-scripts/build/build-pk3-assets.sh
dos2unix /rtcwpro/docker-scripts/build/build-pk3-bin.sh && ./rtcwpro/docker-scripts/build/build-pk3-bin.sh
dos2unix /rtcwpro/docker-scripts/build/build-server.sh && ./rtcwpro/docker-scripts/build/build-server.sh
#move into game dir
# cd game
# make -f makefile
# rm ../*/*.o
# make -f makefile.w32
# rm ../*/*.o
# cd ../cgame
# make -f makefile
# rm ../*/*.o
# make -f makefile.w32
# rm ../*/*.o
# cd ../ui
# make -f makefile
# rm ../*/*.o
# make -f makefile.w32
# rm ../*/*.o
# cd ../
# mv */*.dll ../output/
# mv */*.so ../output/