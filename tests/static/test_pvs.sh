#!/bin/bash
set -eu
# for info see 'read.me'

##############################################################
# Folders structure (05.08.2020)
# NOTE: Drawing will be removed after approval script
##############################################################
# facelift/                 <-- directory with source code
# facelift_pvs/             <-- directory with source code modified by PVS
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
# │  ├─ static/
# │  │   ├─ read.me
# │  │   └─ test_pvs.sh*   <--- executable script

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
DIR_SRC_MODIFIED="$DIR_SRC""_pvs_${DATE_TIME}"
DIR_REPORT="$DIR_BUILD/pvs/report_pvs_${DATE_TIME}"
REPORT_PVS="$DIR_REPORT/report"
LOG_PVS="$DIR_REPORT/project.log"
NUMBER_THREADS="-j 4"
# Free PVS-Studio license is free for open source projects.
# See link in section "Comments for free open source projects": https://www.viva64.com/en/b/0457/
PVS_FREE_LICENSE="// This is an open source non-commercial project. Dear PVS-Studio, please check it.\n// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com"

# Create copy of source code for add strings PVS (free license)
if [ ! -d $DIR_SRC_MODIFIED ]
  then
    cp -r $DIR_SRC $DIR_SRC_MODIFIED
    # Add strings PVS (free license)
    find $DIR_SRC_MODIFIED -type f -name '*.cpp' -exec sed -i "1 i $PVS_FREE_LICENSE" {} \;
fi

# Build facelift with flags
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=On -DFACELIFT_BUILD_TESTS=ON $DIR_SRC_MODIFIED && cmake --build $DIR_BUILD -- $NUMBER_THREADS
mkdir -p $DIR_REPORT
# Run PVS analyzer
pvs-studio-analyzer analyze -o $LOG_PVS $NUMBER_THREADS
plog-converter -a GA:1,2,3 -t fullhtml -o $REPORT_PVS $LOG_PVS
# Show reports
echo "PVS report generated: "$REPORT_PVS
# Need to remove folder with modified source code manually
echo "Need to remove folder with modified source code manually: "$DIR_SRC_MODIFIED
