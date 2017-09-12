#!/bin/bash

set -e

VERSION=0.1

git clone https://github.com/opentracing/opentracing-cpp
cd opentracing-cpp
mkdir .build
cd .build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$THIRDPARTY_BUILD ..
make V=1 install
