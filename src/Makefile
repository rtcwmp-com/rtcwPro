
all: clean dependencies binaries assets models binaries-win resources xhairs
all-valgrind: clean dependencies binaries-valgrind assets models binaries-win resources

dependencies:
	cd .. && bash fetch-dependencies.sh
	mkdir -p ../build
	cp ../deps/libjpeg-turbo/build-win/libjpeg-62.dll ../build
	cp ../deps/curl-win/curl/bin/libcurl.dll ../build

binaries:
	mkdir -p ../build/rtcwpro
	cd game && make all
	cd cgame && make all
	cd ui && make all
	cd unix && make clean && make server && make clean && make client 
	mv game/qagame.mp.i386.so ../build/rtcwpro
	mv cgame/cgame.mp.i386.so ../build/rtcwpro
	mv ui/ui.mp.i386.so ../build/rtcwpro	
	mv unix/wolfded.x86 ../build
	mv unix/wolfMP.x86 ../build
	cd ../build/rtcwpro && zip -r rtcwpro_bin.pk3 ./*.so

binaries-win:
	mkdir -p ../build/objs/ui
	mkdir -p ../build/objs/cgame
	mkdir -p ../build/objs/game
	mkdir -p ../build/objs/client
	mkdir -p ../build/objs/botlib
	mkdir -p ../build/objs/renderer
	mkdir -p ../build/objs/splines
	cd game && make -f makefile.w32 all
	cd cgame && make -f makefile.w32 all
	cd ui && make -f makefile.w32 all
	cd unix && make -f Makefile-win client-w32
	mv unix/wolfMP.exe ../build
	mv unix/wolfMP.pdb ../build
	mv game/qagame_mp_x86.dll ../build/rtcwpro
	mv cgame/cgame_mp_x86.dll ../build/rtcwpro
	mv ui/ui_mp_x86.dll ../build/rtcwpro	
	cd ../build/rtcwpro && zip -r rtcwpro_bin.pk3 ./*.dll

binaries-valgrind:
	cd game && make -f makefile.valgrind all
	cd cgame && make -f makefile.valgrind all
	cd ui && make -f makefile.valgrind all
	cd unix && make -f makefile.valgrind clean && make -f makefile.valgrind server
	mkdir -p ../build/rtcwpro
	mv game/qagame.mp.i386.so ../build/rtcwpro
	mv cgame/cgame.mp.i386.so ../build/rtcwpro
	mv ui/ui.mp.i386.so ../build/rtcwpro	
	mv unix/wolfded.x86 ../build
	cd ../build/rtcwpro && zip -r rtcwpro_bin.pk3 ./*.so


assets:
	cd ../MAIN && zip -r ../build/rtcwpro/rtcwpro_assets.pk3 ./*

xhairs:
	cd ../Assets/xhair && zip -r ../build/rtcwpro/rtcwpro_xhairs.pk3 ./*

models:
	cd ../Assets && mkdir -p models/multiplayer/medpack && cp medpack.jpg models/multiplayer/medpack && zip -r ../build/rtcwpro/rtcwpro_models.pk3 ./models rtcwpro_models.dat && rm -rf ./models

resources:
	cd ../ && mkdir -p build/rtcwpro/configs && cp configs/* build/rtcwpro/configs
	cd ../ && cp GeoIP.dat build/rtcwpro/


clean:
	rm -rf ../build
