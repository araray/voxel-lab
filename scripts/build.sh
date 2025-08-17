#!/usr/bin/env bash
set -euo pipefail

# Usage: ./scripts/build.sh [Release|Debug|RelWithDebInfo|MinSizeRel]
CONFIG="${1:-Release}"

# Options (env overrides):
#   BUILD_DEPS=ON|OFF, BUILD_TESTING=ON|OFF, GENERATOR="Ninja"|"Unix Makefiles"|...
: "${BUILD_DEPS:=ON}"
: "${BUILD_TESTING:=ON}"
: "${GENERATOR:=}"

# Choose a generator if not specified
if [[ -z "${GENERATOR}" ]]; then
  if command -v ninja >/dev/null 2>&1; then
    GENERATOR="Ninja"
  else
    GENERATOR="Unix Makefiles"
  fi
fi

echo "==> Configuring (generator: ${GENERATOR}, config: ${CONFIG}, BUILD_DEPS=${BUILD_DEPS}, BUILD_TESTING=${BUILD_TESTING})"
cmake -S . -B build -G "${GENERATOR}" \
  -DCMAKE_BUILD_TYPE="${CONFIG}" \
  -DBUILD_DEPS="${BUILD_DEPS}" \
  -DBUILD_TESTING="${BUILD_TESTING}"

echo "==> Building"
cmake --build build --config "${CONFIG}" -- -j 4

echo "==> Done. Binary:"
if [[ -f "build/voxel_lab" ]]; then
  echo "  build/voxel_lab"
elif [[ -f "build/Release/voxel_lab.exe" ]]; then
  echo "  build/Release/voxel_lab.exe"
else
  echo "  (see build/ output)"
fi
