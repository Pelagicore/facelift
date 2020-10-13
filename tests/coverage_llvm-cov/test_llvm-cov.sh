#!/bin/bash
set -eu
# for info see 'read.me'

DIR_BUILD=$PWD
DIR_SRC=$(cd $(dirname "$0")/../..; pwd -P)
DIR_GTEST_BUILD="$DIR_BUILD/gtest"
DIR_GTEST_INSTALL="$DIR_GTEST_BUILD/googletest/install"
GTEST_SETUP="$DIR_SRC/ci/travis/setup_gtest.sh"
DIR_REPORT="$DIR_BUILD/llvm-cov/report_$(date +"%y-%m-%d-%H%M%S")"
REPORT_COVERAGE="$DIR_REPORT/index.html"
REPORT_CTEST="$DIR_REPORT/ctest.log"
COVERAGE_COLLECT="${DIR_BUILD}/coverage.info"
COVERAGE_FILTERED="${DIR_BUILD}/coverage_filtered.info"
DATA_PROFILER_OUTPUT="code.profdata"
RAW_PROFILER_OUTPUT="dump-%p.profraw"
EXPORT_FORMAT="lcov"
UNNECESSARY_ENTITIES=( "*/tests/*" "*/examples/*" "*/facelift_generated/*" "*/moc_*.cpp" "*gtest*" )

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
export LLVM_PROFILE_FILE=${RAW_PROFILER_OUTPUT}
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER="clang" -DCMAKE_CXX_COMPILER="clang++" -DCMAKE_EXE_LINKER_FLAGS="-fprofile-instr-generate" -DCMAKE_CXX_FLAGS="-fprofile-instr-generate -fcoverage-mapping" -DFACELIFT_BUILD_TESTS=ON -DCMAKE_SYSTEM_PREFIX_PATH=${CMAKE_SYSTEM_PREFIX_PATH} ${DIR_SRC} && cmake --build ${DIR_BUILD}
mkdir -p $DIR_REPORT
# Run ctest
ctest -VV -O $REPORT_CTEST || true

# Find all *.profraw files
shopt -s lastpipe
FILES_PROFRAW=()
find ${DIR_BUILD} -type f -name "*.profraw" -print0 | while IFS= read -r -d $'\0';
do FILES_PROFRAW+=("$REPLY");
done;
declare -a FILES_PROFRAW
# The llvm-profdata tool is able to get all .profraw files and merge them into a format suitable for llvm-cov.
llvm-profdata merge ${FILES_PROFRAW[@]} -output=${DATA_PROFILER_OUTPUT}

# Find all *.so files
shopt -s lastpipe
FILES_OBJ=()
find ${DIR_BUILD} -type f -executable -name "*.so" -print0 | while IFS= read -r -d $'\0';
do  FILES_OBJ+=("$REPLY");
done;
declare -a FILES_OBJ
# Export coverage data of the binaries using the profile data in LCOV trace file format
# Get counters instrumented in source code (more meaningful data). This requires passing in the built binary or shared library
for FILE_OBJ in ${FILES_OBJ[@]}
do
    llvm-cov export -format=${EXPORT_FORMAT} ${FILE_OBJ} -instr-profile=${DATA_PROFILER_OUTPUT} >> ${COVERAGE_COLLECT}
done

# Remove unnecessary files and folders
lcov \
    --remove ${COVERAGE_COLLECT} \
    ${UNNECESSARY_ENTITIES[@]} \
    --output-file ${COVERAGE_FILTERED}

# Generate HTML view from LCOV coverage data files
genhtml ${COVERAGE_FILTERED} --output-directory ${DIR_REPORT} --legend

# Show reports
echo "ctest report generated: "$REPORT_CTEST
echo "coverage report generated: "$REPORT_COVERAGE
