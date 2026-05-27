#!/bin/bash

echo "threads,time,speedup" > results/threads_benchmark.csv

BASE_TIME=4.89962

for threads in 1 2 3 4 5 6 7 8 10 12 16
do
    export OMP_NUM_THREADS=$threads
    export OMP_SCHEDULE="dynamic,8"

    output=$(./simd)

    time=$(echo "$output" | grep "Tiempo total con SIMD" | awk '{print $5}')

    speedup=$(awk "BEGIN {print $BASE_TIME / $time}")

    echo "$threads,$time,$speedup" >> results/threads_benchmark.csv
done