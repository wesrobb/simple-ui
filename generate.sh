#!/bin/bash
mkdir -p build
cmake -DCMAKE_TOOLCHAIN_FILE=~/src/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-osx -S . -B build 
