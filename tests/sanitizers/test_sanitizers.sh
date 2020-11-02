#!/bin/bash
set -e
# for info see 'read.me'

DIR_BUILD=$PWD
DIR_SRC=$(cd $(dirname "$0")/../..; pwd -P)
DIR_REPORT="$DIR_BUILD/sanitizers/report_$(date +"%y-%m-%d-%H%M%S")"
DIR_GTEST_BUILD="$DIR_BUILD/gtest"
DIR_GTEST_INSTALL="$DIR_GTEST_BUILD/googletest/install"
GTEST_SETUP="$DIR_SRC/ci/travis/setup_gtest.sh"
REPORT_BUILD="$DIR_REPORT/report_build"
REPORT_CTEST="$DIR_REPORT/ctest_log"

# Set a default sanitizer if envvar is not set
if [ -z "$SANITIZER" ]
then
    SANITIZER="ASAN"
    echo "INFO: environment variable SANITIZER is not set. Will be using by default $SANITIZER"
fi

# ASAN: Address Sanitizer
ASAN="-fsanitize=address "
ASAN_FLAGS="-fsanitize-address-use-after-scope -fsanitize-recover=address -fno-omit-frame-pointer -fno-common "
# LSAN: Leak Sanitizer
LSAN="-fsanitize=leak "
LSAN_FLAGS=""
# UBSAN: UndefinedBehaviour Sanitizer
UBSAN="-fsanitize=undefined "
UBSAN_FLAGS="-fno-omit-frame-pointer "
# Compile time out of bounds checks. See more: https://fuzzing-project.org/tutorial-cflags.html
EXTRA_FLAGS="-Wformat -Werror=format-security -Werror=array-bounds "

FLAGS=${SANITIZER}"_FLAGS"
SANITIZER_LDFLAGS="${!SANITIZER}"
SANITIZER_FLAGS="${!SANITIZER}${!FLAGS}"
SANITIZER_FLAGS+="$EXTRA_FLAGS"
SANITIZER_FLAGS+="-shared-libasan "
# NOTE: option '-shared-libasan' for link shared library with:
# ASAN as a shared library. See more: https://github.com/google/sanitizers/wiki/AddressSanitizerAsDso
# UBSAN as a shared library. See more: https://reviews.llvm.org/D38525

CXX="clang++"
CXX_VERSION=$(${CXX} -dM -E -x c /dev/null | grep __clang_version | cut -d" " -f3 | grep -o -E "[[:digit:]]+\.[[:digit:]]+\.[[:digit:]]+")

LIBCLANG_RT_ASAN="libclang_rt.asan-x86_64.so"
LIBCLANG_RT_UBSAN="libclang_rt.ubsan_standalone-x86_64.so"

# Install gtest
if [ ! -d $DIR_GTEST_BUILD ]
  then
    mkdir -p $DIR_GTEST_BUILD
    pushd $DIR_GTEST_BUILD
    # Setup test
    . $GTEST_SETUP
    popd
fi
CMAKE_SYSTEM_PREFIX_PATH="$DIR_GTEST_INSTALL"
# Build facelift with flags
mkdir -p $DIR_REPORT
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER="${CXX}" -DCMAKE_CXX_FLAGS="${SANITIZER_FLAGS}" -DCMAKE_EXE_LINKER_FLAGS="${SANITIZER_LDFLAGS}" -DFACELIFT_BUILD_TESTS=ON -DCMAKE_SYSTEM_PREFIX_PATH=${CMAKE_SYSTEM_PREFIX_PATH} ${DIR_SRC} && cmake --build ${DIR_BUILD} 2>$REPORT_BUILD

# Set sanitizer common flags. See more: https://github.com/google/sanitizers/wiki/SanitizerCommonFlags
case "${SANITIZER}" in
    "ASAN")
       ASAN_OPTIONS="check_initialization_order=1 halt_on_error=false exitcode=0 start_deactivated=true print_cmdline=true quarantine_size_mb=256 detect_leaks=1 verbosity=2+ log_exe_name=1"
    ;;
    "LSAN")
       LSAN_OPTIONS="detect_leaks=1 exitcode=0 max_leaks=0 print_suppressions=1 report_objects=0 use_unaligned=0"
    ;;
    "UBSAN")
       UBSAN_OPTIONS="print_stacktrace=1"
    ;;
    *)
       echo "Sanitizer not specified"
    ;;
esac

# Run tests
case "${SANITIZER}" in
  "ASAN")
    PATH_LIBCLANG_RT=$(locate $LIBCLANG_RT_ASAN | grep clang | grep $CXX_VERSION)
    LD_PRELOAD=${PATH_LIBCLANG_RT} ctest -VV -O $REPORT_CTEST || true
  ;;
  "LSAN")
    PATH_LIBCLANG_RT=$(locate $LIBCLANG_RT_ASAN | grep clang | grep $CXX_VERSION)
    LD_PRELOAD=${PATH_LIBCLANG_RT} ctest -VV -O $REPORT_CTEST || true
  ;;
  "UBSAN")
    PATH_LIBCLANG_RT=$(locate $LIBCLANG_RT_UBSAN | grep clang | grep $CXX_VERSION)
    LD_PRELOAD=${PATH_LIBCLANG_RT} ctest -VV -O $REPORT_CTEST || true
  ;;
  *)
    ctest -VV -O $REPORT_CTEST || true
  ;;
esac

# Show reports
echo "build report with appropriate compile-time checks generated: "$REPORT_BUILD
echo "ctest with sanitizer report generated: "$REPORT_CTEST
