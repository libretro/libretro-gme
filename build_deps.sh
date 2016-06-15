#! /usr/bin/env bash
git submodule init
git submodule update

rm -rf build/*

cd deps/game-music-emu
rm CMakeCache.txt
cmake -G "Unix Makefiles" -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=../../build .
make clean
make
make install
make clean

cd ../libarchive
rm CMakeCache.txt
cmake -G "Unix Makefiles" -DENABLE_CNG=OFF -DCMAKE_INSTALL_PREFIX=../../build .
make clean
make
make install
make clean

cd ../..
