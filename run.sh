#!/bin/sh

mkdir -p bin
g++ -std=c++11 -O3 src/bf.cpp -o bin/bf
g++ -std=c++11 -O3 src/bf-opt.cpp -o bin/bf-opt
g++ -std=c++11 -O3 src/bf-jit.cpp -o bin/bf-jit

echo "--- Default Interpreter ---"
time -p bin/bf < tests/mandelbrot.bf

echo "--- Optimized Interpreter ---"
time -p bin/bf-opt < tests/mandelbrot.bf > mandelbrot

echo "--- JIT Compilation ---"
time -p bin/bf-jit < tests/mandelbrot.bf > mandelbrot

rm mandelbrot
