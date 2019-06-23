#!/bin/bash

num_parallel=7
for i in `seq 1 $num_parallel`;
do
    ./submit.sh $i $num_parallel &
done
