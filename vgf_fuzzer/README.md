VGF Fuzzer
==========

This directory contains a `libFuzzer <https://llvm.org/docs/LibFuzzer.html>`_ harness that exercises both the C and C++
decoder surfaces of VGF lib API. The target is built with AddressSanitizer and UndefinedBehaviorSanitizer so it will crash on
memory-safety bugs and emit useful diagnostics. LeakSanitizer is disabled in the build to avoid false positives in
restricted environments; drop ``-fno-sanitize=leak`` in ``vgf_fuzzer/CMakeLists.txt`` if you want leak checking.

Building
--------

The fuzzer requires Clang/LLVM and the FlatBuffers, Argparse, and JSON for Modern C++ dependencies (either via
``FLATBUFFERS_PATH`` / ``ARGPARSE_PATH`` / ``JSON_PATH`` or installed configs):

.. code-block:: bash

   cmake -S . -B build-fuzz \
     -DCMAKE_C_COMPILER=clang \
     -DCMAKE_CXX_COMPILER=clang++ \
     -DML_SDK_VGF_LIB_BUILD_TOOLS=ON \
     -DARGPARSE_PATH=/path/to/argparse \
     -DFLATBUFFERS_PATH=/path/to/flatbuffers \
     -DJSON_PATH=/path/to/json
   cmake --build build-fuzz --target vgf_fuzzer

Running
-------

Provide a corpus directory (an empty one works; seeds help coverage) and optionally an artifact prefix for crash repros:

.. code-block:: bash

   mkdir -p fuzz_corpus artifacts
   ./build-fuzz/vgf_fuzzer/vgf_fuzzer fuzz_corpus -artifact_prefix=artifacts/ -max_total_time=60

Adjust libFuzzer flags as needed (for example ``-jobs`` and ``-workers`` for parallel fuzzing). The sanitizer runtime is
linked by default; set ``ASAN_OPTIONS`` / ``UBSAN_OPTIONS`` if you need to customize reporting.
