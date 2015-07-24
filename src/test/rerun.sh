#/bin/bash
if [ $# -eq 2 ]; then
make clean
fi
make
./$1
