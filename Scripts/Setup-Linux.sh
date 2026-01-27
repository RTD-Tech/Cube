#!/bin/bash

pushd ..
git submodule update --init --recursive
cmake -S . -B Build
cmake --build Build
popd