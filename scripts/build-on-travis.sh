#!/bin/bash

set -evx

if test "x$BUILD_WITH_CMAKE" = "xyes"; then
  mkdir build
  cd build
  cmake -Wdev -C../cmake/caches/Travis.cmake ..
  cmake --build .
  if test "x$RUN_TESTS" != "xno"; then
    ctest -V
  fi
else
  # Normal build system
  make -f run_configure.mk OMRGLUE=./example/glue SPEC="$SPEC" PLATFORM="$PLATFORM"
  make -j4
  if test "x$RUN_TESTS" != "xno"; then
    make test
  fi
  if test "x$RUN_LINT" = "xyes"; then
    llvm-config --version
    clang++ --version
    make lint
  fi
fi