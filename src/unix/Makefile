# FIXME: add targets for ui/cgame/game + debug targets
# FIXME: basically what cons used - needs tuning
#CFLAGSREL := \
#	-m32 -pipe -march=pentium4 -O3 -fomit-frame-pointer -ffast-math \
#	-falign-loops=2 -falign-jumps=2 -falign-functions=2 \
#	-fno-strict-aliasing -fstrength-reduce -DNDEBUG $(CFLAGSEXT)

CFLAGSREL = -g -pipe -m32 -fsigned-char -DNDEBUG -O6  \
-fomit-frame-pointer -ffast-math -falign-loops=2 -falign-jumps=2 -fcommon \
-falign-functions=2 -fno-strict-aliasing -fstrength-reduce -fPIC -DGAMEDLL 

DCFLAGSREL := \
	 -pipe -m32 -march=pentium4 -g -fomit-frame-pointer -ffast-math \
	-falign-loops=2 -falign-jumps=2 \
	-fno-strict-aliasing -fstrength-reduce -DNDEBUG $(CFLAGSEXT)

%.o : %.nasm
	$(NASM) -f elf -o $@ $<
%.o : %.s
	$(CC) $(CFLAGS) -DELF -x assembler-with-cpp -c -o $@ $<
%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# ../splines/q_shared.cpp seems unneeded and conflicts when compiling 
# all files at once
SPLINEOBJS := \
	../splines/math_angles.o ../splines/math_matrix.o \
	../splines/math_quaternion.o ../splines/math_vector.o \
	../splines/q_parse.o ../splines/splines.o ../splines/util_str.o
BOTLIBOBJS := \
	../botlib/be_aas_bspq3.o ../botlib/be_aas_cluster.o \
	../botlib/be_aas_debug.o ../botlib/be_aas_entity.o \
	../botlib/be_aas_file.o ../botlib/be_aas_main.o \
	../botlib/be_aas_move.o ../botlib/be_aas_optimize.o \
	../botlib/be_aas_reach.o ../botlib/be_aas_route.o \
	../botlib/be_aas_routealt.o ../botlib/be_aas_routetable.o \
	../botlib/be_aas_sample.o ../botlib/be_ai_char.o \
	../botlib/be_ai_chat.o ../botlib/be_ai_gen.o \
	../botlib/be_ai_goal.o ../botlib/be_ai_move.o \
	../botlib/be_ai_weap.o ../botlib/be_ai_weight.o ../botlib/be_ea.o \
	../botlib/be_interface.o ../botlib/l_crc.o ../botlib/l_libvar.o \
	../botlib/l_log.o ../botlib/l_memory.o ../botlib/l_precomp.o \
	../botlib/l_script.o ../botlib/l_struct.o
ASMOBJS := \
	../unix/ftol.o ../unix/snapvector.o
RENDEROBJS := \
	../renderer/tr_animation.o ../renderer/tr_backend.o \
	../renderer/tr_bsp.o ../renderer/tr_cmds.o \
	../renderer/tr_cmesh.o ../renderer/tr_curve.o \
	../renderer/tr_flares.o ../renderer/tr_font.o \
	../renderer/tr_image.o ../renderer/tr_init.o \
	../renderer/tr_light.o ../renderer/tr_main.o \
	../renderer/tr_marks.o ../renderer/tr_mesh.o \
	../renderer/tr_model.o ../renderer/tr_noise.o \
	../renderer/tr_scene.o ../renderer/tr_shade.o \
	../renderer/tr_shade_calc.o ../renderer/tr_shader.o \
	../renderer/tr_shadows.o ../renderer/tr_sky.o \
	../renderer/tr_surface.o ../renderer/tr_world.o \
	../renderer/tr_bloom.o
CLIENTOBJS := \
	../client/cl_cgame.o ../client/cl_cin.o ../client/cl_console.o \
	../client/cl_input.o ../client/cl_keys.o ../client/cl_main.o \
	../client/cl_net_chan.o ../client/cl_parse.o ../client/cl_scrn.o \
	../client/cl_ui.o ../client/cl_events.o ../client/cl_control.o \
	../client/cl_demo.o
COMMONOBJS := \
	../qcommon/cm_load.o ../qcommon/cm_patch.o ../qcommon/cm_polylib.o \
	../qcommon/cm_test.o ../qcommon/cm_trace.o ../qcommon/cmd.o \
	../qcommon/common.o ../qcommon/cvar.o ../qcommon/files.o \
	../qcommon/md4.o ../qcommon/msg.o ../qcommon/net_chan.o \
	../qcommon/huffman.o ../qcommon/unzip.o ../qcommon/db_main.o \
	../qcommon/db_mysql.o ../qcommon/http.o ../qcommon/http_main.o ../qcommon/stats_submit_curl.o \
	../qcommon/api_query.o ../qcommon/json.o ../qcommon/huffman_static.o ../qcommon/md5.o
SOUNDOBJS := \
	../client/snd_adpcm.o ../client/snd_dma.o ../client/snd_mem.o \
	../client/snd_mix.o ../client/snd_wavelet.o
SERVEROBJS := \
	../server/sv_bot.o ../server/sv_ccmds.o ../server/sv_client.o \
	../server/sv_game.o ../server/sv_init.o ../server/sv_main.o ../server/sv_control.o \
	../server/sv_net_chan.o ../server/sv_snapshot.o ../server/sv_animation.o \
	../server/sv_world.o ../server/sv_wallhack.o  ../server/sv_events.o 
# ../qcommon/vm_x86.c is not used and not nice to port to mingw
VMOBJS := \
	../qcommon/vm.o ../qcommon/vm_interpreted.o
SHAREDOBJS := \
	../qcommon/q_shared.o ../qcommon/q_math.o
DEDOBJS := \
	../null/null_client.o ../null/null_input.o ../null/null_snddma.o
UNIXCLIENTOBJS := \
	../unix/linux_qgl.o ../unix/sdl_glimp.o ../unix/sdl_input.o ../unix/sdl_core.o \
	../unix/sdl_snd.o

UNIXOBJS := \
	../unix/linux_common.o ../unix/unix_main.o ../unix/linux_tty.o ../unix/unix_net.o \
	../unix/unix_shared.o ../unix/linux_signals.o ../unix/linux_threads.o
WINCLIENTOBJS := \
	../win32/win_gamma.o ../win32/win_glimp.o ../win32/win_input.o \
	../win32/win_qgl.o ../win32/win_snd.o ../win32/win_threads.o ../win32/win_exception.o
WINOBJS := \
	../win32/win_main.o ../win32/win_net.o ../win32/win_syscon.o \
	../win32/win_wndproc.o ../win32/win_shared.o

#####################################################################

CLOBJS := \
	$(SPLINEOBJS) $(BOTLIBOBJS) $(ASMOBJS) \
	$(CLIENTOBJS) $(COMMONOBJS) $(SOUNDOBJS) $(SERVEROBJS) \
	$(UNIXOBJS) $(UNIXCLIENTOBJS) $(RENDEROBJS) $(VMOBJS) $(SHAREDOBJS)
CLWINOBJS := \
	$(SPLINEOBJS) $(BOTLIBOBJS) $(JPGLIBOBJS) $(RENDEROBJS) $(CLIENTOBJS) \
	$(COMMONOBJS) $(SOUNDOBJS) $(SERVEROBJS) $(WINOBJS) $(WINCLIENTOBJS) \
	$(VMOBJS) $(SHAREDOBJS)
SVOBJS := \
	$(BOTLIBOBJS) $(ASMOBJS) $(SERVEROBJS) $(COMMONOBJS) $(DEDOBJS) \
	$(VMOBJS) $(UNIXOBJS) $(SHAREDOBJS)
SVWINOBJS := \
	$(BOTLIBOBJS) $(SERVEROBJS) $(COMMONOBJS) $(DEDOBJS) $(VMOBJS) \
	$(WINOBJS) $(SHAREDOBJS)

#####################################################################

client: CC := gcc
client: NASM := nasm

client: CFLAGS := $(CFLAGSREL) -m32 -DBOTLIB -DDLL_ONLY -I../../deps/libunwind/build/include/ -I../../deps/curl/build/include/  -I../../deps/libjpeg-turbo/build/ 
client: CPPFLAGS := $(CFLAGS)
client: LDFLAGS := \
	 -m32 -ldl -lm -lstdc++ -lSDL2 -lc
client: $(CLOBJS)
	$(CC) -g -o wolfMP.x86 $(CLOBJS) ../../deps/libunwind/build/lib/libunwind.a ../../deps/curl/build/lib/libcurl.a ../../deps/openssl/build/lib/libssl.a ../../deps/openssl/build/lib/libcrypto.a  ../../deps/libjpeg-turbo/build/libturbojpeg.a $(LDFLAGS)
	#strip -s wolfMP.x86
client-clean:
	rm $(CLOBJS) 2> /dev/null ; exit 0
	
#####################################################################

# This builds the windows client with mingw but mingw doesn't support windows' exception handling
#   may need to exclude win_exception's things to get it to build again, or just don't use it.  
# Makefile-win uses clang-cl to build in msvc mode with Safe Exception Handling support
client-w32: CC := i686-w64-mingw32-gcc-win32
client-w32: CXX := i686-w64-mingw32-g++-win32
client-w32: CFLAGS := \
	$(CFLAGSREL) -g  -DBOTLIB -DDLL_ONLY \
	-D__i386__ -DWIN32 -DSPI_SCREENSAVERRUNNING=SPI_SETSCREENSAVERRUNNING -D__MINGW32__  \
	 -I../../deps/libjpeg-turbo/build-win/ -I../../deps/curl-win/curl/include
client-w32: CPPFLAGS := $(CFLAGS)
client-w32: LDFLAGS := \
	  -g -lwinmm -lopengl32 -lgdi32  -static-libgcc  \
	 `$(CXX) -print-file-name=libstdc++.a` -lws2_32 -lcrypt32 -L../../deps/curl-win/curl/lib -lcurl -mwindows
client-w32: $(CLWINOBJS)
	$(CXX) -o wolfMP.exe $(CLWINOBJS) \
	 ../../deps/libjpeg-turbo/build-win/libturbojpeg.a /usr/i686-w64-mingw32/lib/libversion.a  $(LDFLAGS)
	#strip -s wolfMP.exe
client-w32-clean:
	rm $(CLWINOBJS) 2> /dev/null ; exit 0

#####################################################################

server: CC := gcc
server: NASM := nasm 
server: CFLAGS := \
	$(CFLAGSREL) -m32 -g  -DBOTLIB -DDEDICATED -DC_ONLY -DDLL_ONLY -I../../deps/libunwind/build/include/ -I../../deps/curl/build/include/ 
server: LDFLAGS := -m32 -DCURL_STATICLIB -ldl -lpthread
server: $(SVOBJS)
	$(CC) -o wolfded.x86 $(SVOBJS) ../../deps/libunwind/build/lib/libunwind.a ../../deps/curl/build/lib/libcurl.a ../../deps/openssl/build/lib/libssl.a ../../deps/openssl/build/lib/libcrypto.a $(LDFLAGS)
#	strip -s wolfded.x86
server-clean:
	rm $(SVOBJS) 2> /dev/null ; exit 0
	
server-debug: CC := gcc
server-debug: NASM := nasm
server-debug: CFLAGS := \
	$(DCFLAGSREL) -m32 -DBOTLIB -DDEDICATED -DC_ONLY -DDLL_ONLY
server-debug: LDFLAGS := -m32 -ldl -lm -L..\..\libs\curl-7.29.0\lib\libcurld -lcurl -lmysqlclient --verbose
server-debug: $(SVOBJS)
	$(CC) -o wolfded_debug.x86 $(SVOBJS) $(LDFLAGS)
server-debug-clean:
	rm $(SVOBJS) 2> /dev/null ; exit 0


#####################################################################

# FIXME: is uname -m suitable for CPUSTRING?
# FIXME: had to undef __i386__ to avoid msvc style inline asm - probably
# not the cleanest solution
server-w32: CC := i686-w64-mingw32-gcc-win32
#server-w32: CC := /usr/local/cross-tools/i386-mingw32msvc/bin/gcc
server-w32: CFLAGS := \
	$(CFLAGSREL) -DBOTLIB -DDEDICATED -DC_ONLY -DDLL_ONLY \
	-DCPUSTRING=\"`uname -m`\" -U__i386__  -DWIN32 \
	-DSPI_SCREENSAVERRUNNING=SPI_SETSCREENSAVERRUNNING
server-w32: LDFLAGS := -m32 -lwinmm -lgdi32 -lws2_32 -L..\..\libs\libcurl\lib\libcurld -lcurl -lmysqlclient 
server-w32: $(SVWINOBJS)
	$(CC) -o wolfded.exe $(SVWINOBJS) $(LDFLAGS)
	strip -s wolfded.exe
server-w32-clean:
	rm $(SVWINOBJS) 2> /dev/null ; exit 0

#####################################################################

CURLCONF := \
	"CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32" \
	--prefix=$(PWD)/../../libs/curl-7.75.0 \
	--enable-shared=yes --enable-static=yes --with-ssl --disable-rtsp \
	--disable-ldap --disable-dict --disable-file --disable-gopher \
	--disable-imap --disable-pop3 --disable-smtp --disable-telnet \
	--disable-tftp --without-ca-bundle --without-libidn \
	--without-ca-path --without-nss --without-polarssl \
	--without-gnutls --without-zlib --with-gnu-ld \
	--disable-cookies --disable-tls-srp --disable-ntlm-wb \
	--disable-sspi --disable-verbose  --disable-threaded-resolver \
	--disable-nonblocking --disable-manual 

curl: 
	cd ../../libs/curl-7.75.0 && ./configure $(CURLCONF)
	cd ../../libs/curl-7.75.0 && make
	cp ../../libs/curl-7.75.0/lib/.libs/libcurl.a ../../libs/curl-7.75.0/lib/.libs/libcurl_linux.a
	cp ../../libs/curl-7.75.0/lib/.libs/libcurl.a ../../libs/.libs/libcurl.a
	cp -R ../../libs/curl-7.75.0/include/curl ../../libs/curl-7.75.0/include/curl_linux
	cp -R ../../libs/curl-7.75.0/include/curl ../../libs/include/curl

curl-w32:
	cd ../../libs/libcurl && CC=i586-mingw32msvc-gcc RANLIB=i586-mingw32msvc-ranlib \
	CPPFLAGS=-DCURL_STATICLIB -m32 ./configure $(CURLCONF) --host=mingw32
	cd ../../libs/libcurl && make
	mv ../../libs/libcurllib/.libs/libcurl.a ../../libs/libcurllib/.libs/libcurl_w32.a
	cp -R ../../libs/libcurl/include/curl ../../libs/libcurl/include/curl_w32

curl-clean:
	cd ../../libs/curl-7.75.0 && make clean
#	rm -rf ../../libs/libcurl/include/curl_linux
#	rm -rf ../../libs/libcurl/include/curl_w32

#####################################################################

clean:
	make client-clean
	make client-w32-clean
	make server-clean
	make server-w32-clean
	
all:
	make clean
	make client
	make client-clean
	make server
	make server-clean
	make client-w32
	make client-w32-clean
	make server-w32
	make server-w32-clean

# FIXME: could be more clever
test:
	if [ "$$RTCW_TESTBASE" = "" ]; then \
	RTCW_TESTBASE=~/Software/rtcw ; fi ; \
	cp wolf.x86 "$$RTCW_TESTBASE" ; \
	cp wolf.exe "$$RTCW_TESTBASE" ; \
	cp wolfded.x86 "$$RTCW_TESTBASE" ; \
	cp wolfded.exe "$$RTCW_TESTBASE"
