#!/bin/bash

# Parse input
if [ $# -ne 4 ]; then
    echo "Usage: $0 <# of objects> <max size of objects> <# of tasks> <# of containers>"
    exit
fi

number_of_objects=$1
max_size_of_objects=$2
number_of_processes=$3
number_of_containers=$4

./benchmark/validate $1 $2 $4 < sorted_trace

# if you want to see the log for debugging, comment out the following line.
#rm -f *.log trace sorted_trace

sudo rmmod memory_container

