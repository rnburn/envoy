#!/bin/bash

set -e

VERSION=0.4.0

git clone -b envoy https://github.com/rnburn/lightstep-tracer-cpp-1.git lightstep-tracer-cpp
cd lightstep-tracer-cpp
mkdir .build
cd .build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$THIRDPARTY_BUILD \
  -DBUILD_TESTING=OFF \
  -DENABLE_LINTING=OFF \
  -DWITH_GRPC=OFF
make V=1 install
