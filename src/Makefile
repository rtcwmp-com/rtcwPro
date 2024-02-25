
all: clean dependencies binaries assets models binaries-win

dependencies:
	cd .. && bash fetch-dependencies.sh

binaries:
	cd game && make all
	cd cgame && make all
	cd ui && make all
	cd unix && make clean && make server
	mkdir -p ../build/rtcwpro
	mv game/qagame.mp.i386.so ../build/rtcwpro
	mv cgame/cgame.mp.i386.so ../build/rtcwpro
	mv ui/ui.mp.i386.so ../build/rtcwpro	
	mv unix/wolfded.x86 ../build
	cd ../build/rtcwpro && zip -r rtcwpro_bin.pk3 ./*.so

binaries-win:
	cd game && make -f makefile.w32 all
	cd cgame && make -f makefile.w32 all
	cd ui && make -f makefile.w32 all
	mv game/qagame_mp_x86.dll ../build/rtcwpro
	mv cgame/cgame_mp_x86.dll ../build/rtcwpro
	mv ui/ui_mp_x86.dll ../build/rtcwpro	
	cd ../build/rtcwpro && zip -r rtcwpro_bin.pk3 ./*.dll


assets:
	cd ../MAIN && zip -r ../build/rtcwpro/rtcwpro_assets.pk3 ./*

models:
	cd ../Assets && mkdir -p models/multiplayer/medpack && cp medpack.jpg models/multiplayer/medpack && zip -r ../build/rtcwpro/rtcwpro_models.pk3 ./models rtcwpro_models.dat && rm -rf ./models

clean:
	rm -rf ../deps
	rm -rf ../build
