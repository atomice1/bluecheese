#!/bin/sh
if ! which cmake >/dev/null 2>&1; then
    echo 2>&1 "cmake must be in your PATH"
    exit 1
fi
set -ex
repo="$(readlink -f "$(dirname "$0")")"
workdir="$(mktemp -d)"
cd "${workdir}"
git clone "${repo}"
mkdir build
cd build
cmake ../"$(basename "${repo}")"
cmake --build .
ctest test
rm -rf "${workdir}"
