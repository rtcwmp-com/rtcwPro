#!/bin/bash
mkdir deps
cd deps
VER=$(curl --silent -qI https://github.com/curl/curl/releases/latest | awk -F '/' '/^location/ {print  substr($NF, 1, length($NF)-1)}');
wget https://api.github.com/repos/curl/curl/tarball/$VER
tar xvfz $VER
rm $VER
mv curl* curl
CURL_DIR=`pwd`/curl

VER=$(curl --silent -qI https://github.com/openssl/openssl/releases/latest | awk -F '/' '/^location/ {print  substr($NF, 1, length($NF)-1)}');
wget https://api.github.com/repos/openssl/openssl/tarball/$VER
tar xvfz $VER
rm $VER
mv openssl* openssl
OPENSSL_DIR=`pwd`/openssl

VER=$(curl --silent -qI https://github.com/akheron/jansson/releases/latest | awk -F '/' '/^location/ {print  substr($NF, 1, length($NF)-1)}');
wget https://api.github.com/repos/akheron/jansson/tarball/$VER
tar xvfz $VER
rm $VER
mv *jansson* jansson
JANSSON_DIR=`pwd`/jansson

VER=$(curl --silent -qI https://github.com/libunwind/libunwind/releases/latest | awk -F '/' '/^location/ {print  substr($NF, 1, length($NF)-1)}');
wget https://api.github.com/repos/libunwind/libunwind/tarball/$VER
tar xvfz $VER
rm $VER
mv *libunwind* libunwind
LIBUNWIND_DIR=`pwd`/libunwind


cd $OPENSSL_DIR
mkdir build
mkdir build-win
./Configure --prefix=${OPENSSL_DIR}/build '-Wl,-rpath,$(LIBRPATH)' -m32 linux-x86 no-tests no-docs 
make -j
make install

make clean
./Configure --cross-compile-prefix=i686-w64-mingw32- --prefix=${OPENSSL_DIR}/build-win shared mingw -m32 no-idea no-tests no-docs
make -j
make install


cd $CURL_DIR
mkdir build
mkdir build-win
autoreconf -fi
CFLAGS="-m32" PKG_CONFIG="pkg-config --static" ./configure --disable-shared --enable-static --without-libpsl --without-zlib --with-openssl=${OPENSSL_DIR}/build --prefix=${CURL_DIR}/build
make -j LDFLAGS="-static -all-static"
make install

make clean
autoreconf -fi
CFLAGS="-m32" PKG_CONFIG="pkg-config --static" ./configure --disable-shared --enable-static --without-libpsl --without-zlib --with-openssl=${OPENSSL_DIR}/build-win --prefix=${CURL_DIR}/build-win --target=i686-w64-mingw32 --host=i686-w64-mingw32
make -j LDFLAGS="-static -all-static -L${OPENSSL_DIR}/build-win/lib"
make install


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

cd $LIBUNWIND_DIR
mkdir build
autoreconf -i
export CC="gcc -m32"
export CXX="g++ -m32"
./configure --host=i686-pc-linux-gnu --prefix=${LIBUNWIND_DIR}/build --disable-documentation --disable-tests
make -j
make install
