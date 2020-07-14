#!/bin/bash
set -eu
# for info see 'read.me'

##############################################################
# Folders structure (28.07.2020)
# NOTE: Drawing will be removed after approval script
##############################################################
# facelift/                 <-- directory with source code
# ├─ build/
# │  ├─                     <-- working directory
# │  └─
# ├─ ci/
# │  ├─ appveyor
# │  └─ travis
# ├─ cmake/
# ├─ CMakeLists.txt
# ├─ src/
# ├─ tests/
# │  ├─ asyncfunctions/
# │  ├─ benchmarking/
# │  ├─ cmake/
# │  ├─ CMakeLists.txt
# │  ├─ combined/
# │  ├─ coverage/
# │  │   ├─ read.me
# │  │   └─ test_gcov.sh*   <--- executable script

function outerdir(){
   echo "$1" | sed -E 's@/[^.]+.*@/@'
}
DIR_BUILD=$PWD
DIR_OUTER=$(outerdir "$0")
if [ "$DIR_OUTER" = "./" ]
then
     echo "The script must be run from the 'facelift/build'"
     exit 1
fi
DIR_SRC=$(cd "$DIR_OUTER"; pwd -P)
DATE_TIME=$(date +"%y-%m-%d-%H%M%S")
DIR_REPORT="$DIR_BUILD/gcovr/report_gcovr_${DATE_TIME}"
DIR_CI="$DIR_SRC/ci/travis"
DIR_GTEST="$DIR_BUILD/gtest"
DIR_GTEST_INSTALL="$DIR_GTEST/googletest/install"
GTEST_SETUP="$DIR_CI/setup_gtest.sh"
REPORT_GCOVR="$DIR_REPORT/report.html"
REPORT_CTEST="$DIR_REPORT/ctest.log"
NUMBER_THREADS="-j 4"

# Install gtest
if [ ! -d $DIR_GTEST ]
  then
    mkdir -p $DIR_GTEST
    pushd $DIR_GTEST
    # Setup test
    . $GTEST_SETUP
    popd
fi
CMAKE_SYSTEM_PREFIX_PATH="$DIR_GTEST_INSTALL"
# Build facelift with flags
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXE_LINKER_FLAGS="-lgcov" -DCMAKE_CXX_FLAGS="-fprofile-arcs -ftest-coverage" -DFACELIFT_BUILD_TESTS=ON -DCMAKE_SYSTEM_PREFIX_PATH=$CMAKE_SYSTEM_PREFIX_PATH $DIR_SRC && cmake --build $DIR_BUILD -- $NUMBER_THREADS
mkdir -p $DIR_REPORT
# Run ctest
ctest -VV -O $REPORT_CTEST || true
# Run gcovr
gcovr -r ../ . $NUMBER_THREADS --exclude-directories="^tests" --exclude-directories="^ci" --exclude-directories="^examples" --html --html-details -o $REPORT_GCOVR
# Show reports
echo "Ctest report generated: "$REPORT_CTEST
echo "Gcovr report generated: "$REPORT_GCOVR
