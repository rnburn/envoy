#!/bin/bash

set -e

VERSION=1.0.0

wget -O opentracing-cpp-$VERSION.tar.gz https://github.com/opentracing/opentracing-cpp/archive/v${VERSION}.tar.gz
tar xf opentracing-cpp-$VERSION.tar.gz
cd opentracing-cpp-$VERSION

mkdir .build
cd .build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$THIRDPARTY_BUILD ..
make V=1 install
