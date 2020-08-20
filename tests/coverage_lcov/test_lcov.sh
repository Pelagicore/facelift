#!/bin/bash
set -eu
# for info see 'read.me'

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
DIR_CI="$DIR_SRC/ci/travis"
DIR_GTEST="$DIR_BUILD/gtest"
DIR_GTEST_INSTALL="$DIR_GTEST/googletest/install"
GTEST_SETUP="$DIR_CI/setup_gtest.sh"
DATE_TIME=$(date +"%y-%m-%d-%H%M%S")
DIR_REPORT="$DIR_BUILD/lcov/report_coverage_${DATE_TIME}"
REPORT_COVERAGE="$DIR_REPORT/index.html"
REPORT_CTEST="$DIR_REPORT/ctest.log"
COVERAGE_FILE="${DIR_BUILD}/coverage.info"
COVERAGE_OUT_FILE="${DIR_BUILD}/coverage_out.info"
LLVM_COV_GCOV_PATCH="llvm-gcov.sh"

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
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXE_LINKER_FLAGS="-lgcov --coverage" -DCMAKE_CXX_FLAGS="-fprofile-arcs -ftest-coverage" -DFACELIFT_BUILD_TESTS=ON -DCMAKE_SYSTEM_PREFIX_PATH=$CMAKE_SYSTEM_PREFIX_PATH $DIR_SRC && cmake --build ${DIR_BUILD}
mkdir -p $DIR_REPORT
# Run ctest
ctest -VV -O $REPORT_CTEST || true
# Read the *.gcda with llvm-cov
# NOTE: about all options see: http://www.tin.org/bin/man.cgi?section=1&topic=LLVM-COV
llvm-cov gcov --function-summaries --branch-probabilities *.gcda
# Check availability 'llmv-gcov.sh'
if [ -z $(which ${LLVM_COV_GCOV_PATCH}) ]
then
    echo "The 'llvm-cov' command line options have been changed, and 'lcov' can no longer recognize it"
    echo "Please create a script '/usr/bin/llvm-gcov.sh' to workaround the problem."
    echo "#!/bin/bash"
    echo "exec llvm-cov gcov \"$@\""
    echo "$ chmod +x llvm-gcov.sh"
    exit 1
fi
# Collect the code coverage results
lcov --no-external --directory ${DIR_BUILD} --base-directory ${DIR_SRC} --gcov-tool ${LLVM_COV_GCOV_PATCH} --capture \
    --output-file ${COVERAGE_FILE}
# Remove unnecessary files and directories
lcov \
    --remove ${COVERAGE_FILE} \
    "*/tests/*" \
    "*/examples/*"  \
    "*/facelift_generated/*" \
    "*/moc_*.cpp" \
    "*gtest*" \
    --output-file ${COVERAGE_OUT_FILE}
# Generate HTML view from LCOV coverage data files
genhtml ${COVERAGE_OUT_FILE} --output-directory ${DIR_REPORT} --legend

# Show reports
echo "ctest report generated: "$REPORT_CTEST
echo "lcov report generated: "$REPORT_COVERAGE
