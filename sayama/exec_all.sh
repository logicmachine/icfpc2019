#!/bin/bash

num_parallel=7
start_id=1
end_id=300

for i in `seq 1 $num_parallel`;
do
    offset=$(( i + start_id ))
    ./exec.sh $offset $num_parallel $end_id &
done

wait
