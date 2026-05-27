#!/bin/bash

export OMP_NUM_THREADS=8

echo "scheduler,time" > results/benchmark.csv

for sched in \
"static" \
"dynamic,4" \
"dynamic,8" \
"dynamic,16" \
"guided,4" \
"guided,8" \
"guided,16"
do

    export OMP_SCHEDULE=$sched

    output=$(./simd)

    time=$(echo "$output" | grep "Tiempo total con SIMD" | awk '{print $5}')

    echo "$sched,$time" >> results/benchmark.csv

done