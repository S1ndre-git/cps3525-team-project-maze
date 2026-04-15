#!/usr/bin/env bash
set -euo pipefail

CXX="${CXX:-g++}"
STD="${STD:--std=c++11}"
FLAGS="${FLAGS:--Wall -Wextra -O2}"

echo "Using: ${CXX} ${STD} ${FLAGS}"

"${CXX}" ${STD} ${FLAGS} -c get_validate_input.cpp -o get_validate_input.o
"${CXX}" ${STD} ${FLAGS} -c function.cpp -o function.o
"${CXX}" ${STD} ${FLAGS} -c main.cpp -o main.o
"${CXX}" ${STD} ${FLAGS} get_validate_input.o function.o main.o -o maze.cgi

echo "Built: maze.cgi (chmod +x and place in cgi-bin for Apache)"
