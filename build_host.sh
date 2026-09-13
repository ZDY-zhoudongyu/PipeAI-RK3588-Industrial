#!/usr/bin/env bash
set -euo pipefail
cmake -S . -B build/host -DPIPEAI_BUILD_TESTS=ON -DPIPEAI_WITH_RKNN=OFF
cmake --build build/host -j"$(nproc)"
ctest --test-dir build/host --output-on-failure
