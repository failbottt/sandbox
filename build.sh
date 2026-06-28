#!/bin/sh

set -e

CC="gcc"

if [ -d "./build" ]; then
    rm -rf ./build
fi

mkdir ./build

for arg in "$@"; do
    echo "Building: $arg"

    if [ "$arg" = "triangle" ]; then
        $CC -g ./src/triangle.c $FILES -o ./build/triangle -lX11 -lGL
    fi

    if [ "$arg" = "sandbox" ]; then
        $CC -g ./src/sandbox.c $FILES -o ./build/sandbox \
        -lX11 \
        -lGL \
        -lm \
        -lXi \
        -lXfixes
    fi
done
