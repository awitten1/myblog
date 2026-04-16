#!/bin/bash

g++ experiments.cpp -o lap -O2 -std=c++11

./lap $((1 << 20)) > results.csv
