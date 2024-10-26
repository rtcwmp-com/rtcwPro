#!/bin/bash

CMAKEMINGW=$(cat <<EOF
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR X86)
set(CMAKE_C_COMPILER /usr/bin/i686-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/i686-w64-mingw32-g++)
set(CMAKE_RC_COMPILER /usr/bin/i686-w64-mingw32-windres)
set(CMAKE_SHARED_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
EOF
)

CMAKECLANGCL=$(cat <<EOF
set(CMAKE_C_COMPILER clang-cl)
set(CMAKE_CXX_COMPILER clang-cl)
set(MSVC_VERSION 1300)
set(MSVC_INCREMENTAL_DEFAULT ON)
add_compile_options(-fuse-ld=lld-link)
add_definitions(-DWIN32 -DNOMINMAX -DWIN32_LEAN_AND_MEAN -D_XKEYCHECK_H -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
-D_WIN32_WINNT=0x0600 -DUNICODE -D_UNICODE)
add_compile_options(-fms-extensions -fms-compatibility -Wno-ignored-attributes
					-Wno-unused-local-typedef
				    -Wno-expansion-to-defined -Wno-pragma-pack -Wno-ignored-pragma-intrinsic
					-Wno-unknown-pragmas -Wno-invalid-token-paste -Wno-deprecated-declarations -Wno-macro-redefined
					-Wno-dllimport-static-field-def
					-Wno-unused-command-line-argument
					-Wno-unknown-argument
					-Wno-int-to-void-pointer-cast)
EOF
)


mkdir -p deps
cd deps
DEPS_ROOT=`pwd`

OPENSSL_DIR=`pwd`/openssl
if [ ! -d "$OPENSSL_DIR" ]; then
VER=$(curl --silent -qI https://github.com/openssl/openssl/releases/latest | awk -F '/' '/^location/ {print  substr($NF, 1, length($NF)-1)}');
wget https://api.github.com/repos/openssl/openssl/tarball/$VER
tar xvfz $VER
rm $VER
mv openssl* openssl
cd $OPENSSL_DIR
mkdir build
./Configure --prefix=${OPENSSL_DIR}/build '-Wl,-rpath,$(LIBRPATH)' -m32 linux-x86 no-tests no-docs 
make -j
make install
fi
cd $DEPS_ROOT

CURL_DIR=`pwd`/curl
if [ ! -d "$CURL_DIR" ]; then
VER=$(curl --silent -qI https://github.com/curl/curl/releases/latest | awk -F '/' '/^location/ {print  substr($NF, 1, length($NF)-1)}');
wget https://api.github.com/repos/curl/curl/tarball/$VER
tar xvfz $VER
rm $VER
mv curl-curl* curl
cd $CURL_DIR
mkdir build
autoreconf -fi
CFLAGS="-m32" PKG_CONFIG="pkg-config --static" ./configure --disable-shared --enable-static --without-libpsl --without-zlib --with-openssl=${OPENSSL_DIR}/build --prefix=${CURL_DIR}/build
make -j LDFLAGS="-static -all-static"
make install
fi
cd $DEPS_ROOT



JANSSON_DIR=`pwd`/jansson
if [ ! -d "$JANSSON_DIR" ]; then
VER=$(curl --silent -qI https://github.com/akheron/jansson/releases/latest | awk -F '/' '/^location/ {print  substr($NF, 1, length($NF)-1)}');
wget https://api.github.com/repos/akheron/jansson/tarball/$VER
tar xvfz $VER
rm $VER
mv *jansson* jansson
cd $JANSSON_DIR
mkdir build
mkdir build-win
autoreconf -i
CFLAGS="-m32" ./configure --prefix=${JANSSON_DIR}/build
make -j
make install

autoreconf -i
CFLAGS="-m32" ./configure --prefix=${JANSSON_DIR}/build-win --target=i686-w64-mingw32 --host=i686-w64-mingw32
make -j
make install
cd $JANSSON_DIR
cd build-win
gendef libjansson-4.dll
i686-w64-mingw32-dlltool -d libjansson-4.def -l libjansson-4.lib
fi
cd $DEPS_ROOT

LIBUNWIND_DIR=`pwd`/libunwind
if [ ! -d "$LIBUNWIND_DIR" ]; then
VER=$(curl --silent -qI https://github.com/libunwind/libunwind/releases/latest | awk -F '/' '/^location/ {print  substr($NF, 1, length($NF)-1)}');
wget https://api.github.com/repos/libunwind/libunwind/tarball/$VER
tar xvfz $VER
rm $VER
mv *libunwind* libunwind
cd $LIBUNWIND_DIR
mkdir build
autoreconf -i
export CC="gcc -m32"
export CXX="g++ -m32"
./configure --host=i686-pc-linux-gnu --prefix=${LIBUNWIND_DIR}/build --disable-documentation --disable-tests
make -j
make install
fi
cd $DEPS_ROOT

LIBSDL_DIR=`pwd`/SDL
if [ ! -d "$LIBSDL_DIR" ]; then
VER=$(curl --silent -qI https://github.com/libsdl-org/SDL/releases/latest | awk -F '/' '/^location/ {print  substr($NF, 1, length($NF)-1)}');
wget https://api.github.com/repos/libsdl-org/SDL/tarball/$VER
tar xvfz $VER
rm $VER
mv *SDL* SDL
cd $LIBSDL_DIR
mkdir build
mkdir build-win
./autogen.sh
./configure --build=i686-pc-linux-gnu "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32" --prefix=${LIBSDL_DIR}/build 
make -j
make install
fi
cd $DEPS_ROOT

LIBJPEG_DIR=`pwd`/libjpeg-turbo
if [ ! -d "$LIBJPEG_DIR" ]; then
VER=$(curl --silent -qI https://github.com/libjpeg-turbo/libjpeg-turbo/releases/latest | awk -F '/' '/^location/ {print  substr($NF, 1, length($NF)-1)}');
wget https://api.github.com/repos/libjpeg-turbo/libjpeg-turbo/tarball/$VER
tar xvfz $VER
rm $VER
mv *libjpeg-turbo* libjpeg-turbo
cd $LIBJPEG_DIR
mkdir build
mkdir build-win
cd build
CFLAGS=-m32 CXXFLAGS=-m32 LDFLAGS=-m32  cmake -G"Unix Makefiles" ..
make -j
cp ../*.h .

cd ../build-win
echo "${CMAKEMINGW}" > toolchain.cmake
cmake -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=./toolchain.cmake  -DCMAKE_INSTALL_PREFIX=${LIBJPEG_DIR}/build-win ..
LDFLAGS="-static -static-libgcc -static-libstdc++" make -j
cp ../*.h .
gendef libjpeg-62.dll
i686-w64-mingw32-dlltool -d libjpeg-62.def -l libjpeg-62.lib
fi
cd $DEPS_ROOT

XWIN_DIR=`pwd`/xwin
if [ ! -d "$XWIN_DIR" ]; then
VER=$(curl --silent -qI https://github.com/Jake-Shadle/xwin/releases/latest | awk -F '/' '/^location/ {print  substr($NF, 1, length($NF)-1)}');
xwin_prefix="xwin-$VER-x86_64-unknown-linux-musl"
wget https://github.com/Jake-Shadle/xwin/releases/download/$VER/$xwin_prefix.tar.gz
tar xvfz $xwin_prefix.tar.gz
rm $xwin_prefix.tar.gz
mv *xwin* xwin

cd $XWIN_DIR
while : ; do
	./xwin --arch x86 --accept-license download
	./xwin --arch x86 --accept-license unpack
	./xwin --arch x86 --accept-license splat --output .
	ret=$?
	[[ $ret -ne 0 ]] || break
done
fi
cd $DEPS_ROOT

CURLWIN_DIR=`pwd`/curl-win
if [ ! -d "$CURLWIN_DIR" ]; then
#git clone https://github.com/curl/curl-for-win/
#cd $CURLWIN_DIR
#CURLWIN_REV=`git log -n1 --format="%H"`
#export CW_CONFIG=main-win
#export CW_REVISION="${CURLWIN_REV}"
#. ./_versions.sh
#docker trust inspect --pretty "${DOCKER_IMAGE}"
#docker pull "${DOCKER_IMAGE}"
#docker images --digests
#if [ -z "${DND_ENV}" ]; then
#	BUILD_ENV=$(pwd)
#else
#	BUILD_ENV=${DND_ENV}
#fi
#docker run --volume "${BUILD_ENV}:${BUILD_ENV}" --workdir "${BUILD_ENV}" \
#            --env-file <(env | grep -a -E '^(CW_|GITHUB_|DO_NOT_TRACK)') \
#            "${DOCKER_IMAGE}" \
#            sh -c ./_ci-linux-debian.sh
mkdir curl-win
cd curl-win
wget https://curl.se/windows/latest.cgi?p=win32-mingw.zip
mv *win32-mingw.zip curl.zip
unzip curl.zip
rm curl.zip
mv curl*win32-mingw curl
cd curl/bin
gendef libcurl.dll
i686-w64-mingw32-dlltool -d libcurl.def -l libcurl.lib
fi













