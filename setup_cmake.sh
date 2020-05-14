#! /bin/bash

CMAKE_VERSION=3.17.2

CMAKE_URL="https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION.tar.gz"
mkdir "cmake-$CMAKE_VERSION"
travis_retry wget --no-check-certificate -O - $CMAKE_URL | tar --strip-components=1 -xz -C cmake-$CMAKE_VERSION
cd cmake-$CMAKE_VERSION
./bootstrap
make && sudo make install && cd ..
rm -rf cmake-$CMAKE_VERSION.tar.gz
export PATH="$PWD/cmake-$CMAKE_VERSION/bin:$PATH"

