#!/bin/bash   
set -x

make clean
make

./model -z 10 -x 10 -y 10 -v 10 -w 10 -f "result.txt"