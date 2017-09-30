#!/bin/bash

set -e

VERSION=0.4

git clone https://github.com/rnburn/lightstep-tracer-cpp-1.git lightstep-tracer-cpp
# git clone https://github.com/lightstep/lightstep-tracer-common.git lightstep-tracer-cpp/lightstep-tracer-common/
cd lightstep-tracer-cpp
mkdir .build
cd .build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$THIRDPARTY_BUILD -DLS_WITH_GRPC=OFF ..
make V=1 install

# wget -O lightstep-tracer-cpp-$VERSION.tar.gz https://github.com/lightstep/lightstep-tracer-cpp/releases/download/v${VERSION//./_}/lightstep-tracer-cpp-$VERSION.tar.gz
# tar xf lightstep-tracer-cpp-$VERSION.tar.gz
# cd lightstep-tracer-cpp-$VERSION

# # see https://github.com/lyft/envoy/issues/1387 for progress
# cat > ../lightstep-missing-header.diff << EOF
# --- ./src/c++11/lightstep/options.h.bak	2017-08-04 09:30:19.527076744 -0400
# +++ ./src/c++11/lightstep/options.h	2017-08-04 09:30:33.742106924 -0400
# @@ -5,6 +5,7 @@
#  // Options for Tracer implementations, starting Spans, and finishing
#  // Spans.
