#!/bin/bash
set -e
make
echo "Running main.out"
./build/main.out
