#!/bin/sh

#move to src dir
cd /ospx/src

#move into game dir
cd game
make -f makefile
rm ../*/*.o
make -f makefile.w32
rm ../*/*.o
cd ../cgame
make -f makefile
rm ../*/*.o
make -f makefile.w32
rm ../*/*.o
cd ../ui
make -f makefile
rm ../*/*.o
make -f makefile.w32
rm ../*/*.o
cd ../
mv */*.dll ../output/
mv */*.so ../output/
