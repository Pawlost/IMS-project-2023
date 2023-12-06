#!/bin/bash   
set -x

make clean
make

# PRVNI EXPERIMENT
./model -z 1 -x 15 -y 30 -v 10 -w 10 -f "result11.txt"
./model -z 2 -x 15 -y 30 -v 10 -w 10 -f "result12.txt"
./model -z 3 -x 15 -y 30 -v 10 -w 10 -f "result13.txt"
./model -z 4 -x 15 -y 30 -v 10 -w 10 -f "result14.txt"
./model -z 5 -x 15 -y 30 -v 10 -w 10 -f "result15.txt"

# DRUHY EXPERIMENT
./model -z 1 -x 2 -y 4 -v 1 -w 10 -f "result21.txt"