#!/usr/bin/env bash
set -euo pipefail
: "${RKNN_SDK_ROOT:?Set RKNN_SDK_ROOT to Rockchip rknpu2 runtime/Linux/librknn_api directory}"
cmake -S . -B build/rk3588 \
  -DCMAKE_TOOLCHAIN_FILE=toolchain-rk3588.cmake \
  -DPIPEAI_BUILD_TESTS=OFF \
  -DPIPEAI_WITH_RKNN=ON \
  -DRKNN_SDK_ROOT="$RKNN_SDK_ROOT"
cmake --build build/rk3588 -j"$(nproc)"
