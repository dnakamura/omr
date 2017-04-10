#!/bin/bash

set -evx

if test "x$BUILD_WITH_CMAKE" = "xyes"; then
  mkdir build
  cd build
  cmake -Wdev -C../cmake/caches/Travis.cmake ..
  if test "x$RUN_BUILD" != "xno"; then
    cmake --build .
    if test "x$RUN_TESTS" != "xno"; then
      ctest -V
    fi
  fi
else
  make -f run_configure.mk OMRGLUE=./example/glue SPEC="$SPEC" PLATFORM="$PLATFORM"
  if test "x$RUN_BUILD" != "xno"; then
    # Normal build system
    make -j4
    if test "x$RUN_TESTS" != "xno"; then
      make test
    fi
  fi
  if test "x$RUN_LINT" = "xyes"; then
    llvm-config --version
    clang++ --version
    make lint
  fi
fi
