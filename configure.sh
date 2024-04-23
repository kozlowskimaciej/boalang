#!/usr/bin/bash

[ ! -d "venv" ] && python3 -m venv venv
source venv/bin/activate
pip install -r ./requirements.txt

BUILD_TYPE=${1:-Debug}

conan profile detect
conan install . --build=missing --settings=build_type="${BUILD_TYPE}" -s compiler.cppstd=gnu20

./build.sh "${BUILD_TYPE}"
