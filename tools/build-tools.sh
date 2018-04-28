#!/bin/bash
# Usage:
# $ mkdir build
# $ cd build
# $ build-tools.sh
# $ make -j4

realpath() {
    [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

BUILD_FOLDER=$PWD
INSTALL_FOLDER=$PWD/install
THIS_FOLDER=`dirname $0`
SOURCE_FOLDER=`realpath ${THIS_FOLDER}`

echo Installed software will be located in $INSTALL_FOLDER
cmake $SOURCE_FOLDER -DCMAKE_INSTALL_PREFIX=$INSTALL_FOLDER -DCMAKE_PREFIX_PATH=$SOURCE_FOLDER/multimake -DWITH_CCACHE=ON -DCMAKE_BUILD_TYPE=Debug "$@"
echo "Use the \"make -jX\" command to start building the packages"
