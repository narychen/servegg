#!/bin/bash
g++ -g -O0 -L../../src/lib -I../../src/include -std=c++11 -Wall -Wno-deprecated -Wno-unused-function -Wno-unused-variable -fdiagnostics-color=always $1 -levent 
if [ $? -eq 0 ]
then
  ./a.out
fi