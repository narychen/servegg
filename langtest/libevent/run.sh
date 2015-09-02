#!/bin/bash
g++ -g -O0 -I../../src/base -I../../src/include -I../../src/include/hiredis -I../../src/include/event2 -std=c++11 -Wall -Wno-deprecated -Wno-unused-function -Wno-unused-variable -fdiagnostics-color=always $1 -lbase -llog4cxx -levent -lhiredis -L../../src/lib -Wl,-rpath,../../src/lib
if [ $? -eq 0 ]
then
  ./a.out
fi