#!/bin/bash
set -eu
# for info see 'read.me'

DIR_BUILD=$PWD
DIR_SRC=$(cd $(dirname "$0")/../..; pwd -P)
DIR_REPORT="$DIR_BUILD/clang-tidy/report_$(date +"%y-%m-%d-%H%M%S")"
REPORT_ANALYZER="$DIR_REPORT/report"
# Use -checks=* to see all available checks. See more settings: https://clang.llvm.org/extra/clang-tidy/
CHECKS="-checks=*,-fuchsia-*"
CLANG_TIDY_FLAGS="clang-tidy;-extra-arg=-std=c++14;-header-filter=.*;-p=${DIR_BUILD};${CHECKS}"
# Build facelift with flags
mkdir -p $DIR_REPORT
cmake -DCMAKE_CXX_CLANG_TIDY=${CLANG_TIDY_FLAGS} -DFACELIFT_BUILD_TESTS=ON ${DIR_SRC} && cmake --build ${DIR_BUILD} 2>$REPORT_ANALYZER
# Show reports
echo "clang-tidy report generated: "$REPORT_ANALYZER
