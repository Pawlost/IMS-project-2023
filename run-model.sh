#!/bin/bash   
set -x

make clean
make

# prvni mereni
./model -z 1 -x 15 -y 30 -v 1 -w 10 -f "result1.txt"
./model -z 2 -x 15 -y 30 -v 1 -w 10 -f "result2.txt"
./model -z 3 -x 15 -y 30 -v 1 -w 10 -f "result3.txt"
./model -z 4 -x 15 -y 30 -v 1 -w 10 -f "result4.txt"
./model -z 5 -x 15 -y 30 -v 1 -w 10 -f "result5.txt"