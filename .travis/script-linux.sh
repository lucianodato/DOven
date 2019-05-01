#!/bin/bash

set -e

# Preparation
_FLAGS="-Werror"
export CFLAGS="${_FLAGS}"
export CXXFLAGS="${_FLAGS}"

# Start clean
make clean >/dev/null

# Build now
make

# Compress build in a zip file
zip -9 -r doven-$TRAVIS_TAG-linux.zip ./bin