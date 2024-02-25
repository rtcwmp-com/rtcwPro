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


cd $OPENSSL_DIR
mkdir build
./Configure --prefix=${OPENSSL_DIR}/build '-Wl,-rpath,$(LIBRPATH)' -m32 linux-x86 no-tests no-docs 
make -j
make install

cd $CURL_DIR
mkdir build
autoreconf -fi
CFLAGS="-m32" PKG_CONFIG="pkg-config --static" ./configure --disable-shared --enable-static --without-libpsl --without-zlib --with-openssl=${OPENSSL_DIR}/build --prefix=${CURL_DIR}/build
make -j LDFLAGS="-static -all-static"
make install

cd $JANSSON_DIR
mkdir build
autoreconf -i
CFLAGS="-m32" ./configure --prefix=${JANSSON_DIR}/build
make -j
make install
