
all: clean dependencies binaries assets models

dependencies:
	cd .. && fetch-dependencies.sh

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


assets:
	cd ../MAIN && zip -r ../build/rtcwpro/rtcwpro_assets.pk3 ./*

models:
	cd ../Assets && mkdir -p models/multiplayer/medpack && cp medpack.jpg models/multiplayer/medpack && zip -r ../build/rtcwpro/rtcwpro_models.pk3 ./models rtcwpro_models.dat && rm -rf ./models

clean:
	rm -rf ../deps
	rm -rf ../build
