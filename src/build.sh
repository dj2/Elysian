#!/bin/bash

CLANG=clang++
CFLAGS="-std=c++20 -Wall  -Wextra -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -pedantic-errors -I$VULKAN_SDK/include"

CFLAGS="$CFLAGS -Wno-poison-system-directories"

set -x

rm engine.pcm a.out

# $CLANG $CFLAGS \
#   -fmodules-ts \
#   --precompile -x c++-module \
#   -o engine.pcm \
#   engine.cc

# $CLANG $CFLAGS \
#   -fmodules-ts \
#   -fprebuilt-module-path=. \
#   engine.pcm \
#   main.cc

$CLANG $CFLAGS \
  -fmodules \
  --precompile -x c++-module \
  -o engine.pcm \
  engine.cc

$CLANG $CFLAGS \
  -fmodules-ts \
  -fprebuilt-module-path=. \
  engine.pcm \
  main.cc
