#!/usr/bin/env bash

[ ! -d "venv" ] && python3 -m venv venv
source venv/bin/activate
pip install -U conan

conan profile detect
conan install . --build=missing --settings=build_type=Debug

./build.sh