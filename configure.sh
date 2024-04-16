#!/usr/bin/env bash

[ ! -d "venv" ] && python3 -m venv venv
source venv/bin/activate
pip install -U conan

BUILD_TYPE=${1:-Debug}

conan profile detect
conan install . --build=missing --settings=build_type="${BUILD_TYPE}" -s compiler.cppstd=gnu20

./build.sh "${BUILD_TYPE}"
