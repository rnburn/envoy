.. _install_requirements:

Requirements
============

Envoy was initially developed and deployed on Ubuntu 14 LTS. It should work on any reasonably
recent Linux including Ubuntu 16 LTS.

Envoy has the following requirements:

* GCC 4.9+ (for C++11 regex support)
* `Bazel <https://github.com/bazelbuild/bazel>`_ (last tested with 0.5.3)
* `spdlog <https://github.com/gabime/spdlog>`_ (last tested with 0.13.0)
* `http-parser <https://github.com/nodejs/http-parser>`_ (last tested with 2.7.1)
* `nghttp2 <https://github.com/nghttp2/nghttp2>`_ (last tested with 1.23.1)
* `libevent <http://libevent.org/>`_ (last tested with 2.1.8)
* `tclap <http://tclap.sourceforge.net/>`_ (last tested with 1.2.1)
* `gperftools <https://github.com/gperftools/gperftools>`_ (last tested with 2.6.1)
* `BoringSSL <https://boringssl.googlesource.com/boringssl>`_ (last tested with sha 68f84f5c40644e029ed066999448696b01caba7a).
* `protobuf <https://github.com/google/protobuf>`_ (last tested with 3.4.0)
* `opentracing-cpp <https://github.com/opentracing/opentracing-cpp>`_ (last tested with 0.1)
* `lightstep-tracer-cpp <https://github.com/lightstep/lightstep-tracer-cpp/>`_ (last tested with 0.36)
* `rapidjson <https://github.com/miloyip/rapidjson/>`_ (last tested with 1.1.0)
* `c-ares <https://github.com/c-ares/c-ares>`_ (last tested with 1.13.0)
* `backward <https://github.com/bombela/backward-cpp>`_ (last tested with 1.3)
* `zlib <https://github.com/madler/zlib>`_ (last tested with 1.2.11)
* `yaml-cpp <https://github.com/jbeder/yaml-cpp>`_ (last tested with sha e2818c423e5058a02f46ce2e519a82742a8ccac9).

In order to compile and run the tests the following is required:

* `googletest <https://github.com/google/googletest>`_ (last tested with 1.8.0)

In order to run code coverage the following is required:

* `gcovr <http://gcovr.com/>`_ (last tested with 3.3)
