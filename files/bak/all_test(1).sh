#!/bin/bash

rm -f *.log
rm -f trace
rm -f sorted_trace
cd kernel_module
sudo make
sudo make install
cd ..
echo > log.txt
echo "test 1" &>> log.txt
rm -f *.log
rm -f trace
rm -f sorted_trace
bash test.sh 128 1024 1 1 &>> log.txt

echo "test 2" &>> log.txt
rm -f *.log
rm -f trace
rm -f sorted_trace
bash test.sh 256 4096 1 1 &>> log.txt

echo "test 3" &>> log.txt
rm -f *.log
rm -f trace
rm -f sorted_trace
bash test.sh 256 4096 8 2 &>> log.txt

echo "test 4" &>> log.txt
rm -f *.log
rm -f trace
rm -f sorted_trace
bash test.sh 256 8192 64 64 &>> log.txt

echo "test 5" &>> log.txt
rm -f *.log
rm -f trace
rm -f sorted_trace
bash test.sh 256 16384 64 16 &>> log.txt

echo "test 6" &>> log.txt
rm -f *.log
rm -f trace
rm -f sorted_trace
bash test.sh 128 2048 256 256 &>> log.txt

echo "test 7" &>> log.txt
rm -f *.log
rm -f trace
rm -f sorted_trace
bash test.sh 128 2048 256 64 &>> log.txt

echo "test 8" &>> log.txt
rm -f *.log
rm -f trace
rm -f sorted_trace
bash test.sh 16384 2048 8 4 &>> log.txt

echo "all test finished"
cat log.txt
