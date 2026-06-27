#!/bin/sh

set -e

gcc -g main.c -o exe -lX11 -lGL
