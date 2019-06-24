#!/bin/bash

num_parallel=7
start_id=$1
end_id=$2

for i in `seq 1 $num_parallel`;
do
    offset=$(( i + start_id ))
    ./submit.sh $offset $num_parallel $end_id &
done
