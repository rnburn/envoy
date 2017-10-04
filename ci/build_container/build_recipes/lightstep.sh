#!/bin/bash

set -e

VERSION=0.4.0

git clone -b v$VERSION https://github.com/lightstep/lightstep-tracer-cpp.git
cd lightstep-tracer-cpp
mkdir .build
cd .build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$THIRDPARTY_BUILD \
  -DBUILD_TESTING=OFF \
  -DENABLE_LINTING=OFF \
  -DLS_WITH_GRPC=OFF
make V=1 install
