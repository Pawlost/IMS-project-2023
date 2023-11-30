#!/bin/bash   
set -x

make
./model -r 10 -a 19 -b 10 -f "result.txt"