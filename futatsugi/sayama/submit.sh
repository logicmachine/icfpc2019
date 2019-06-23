#!/bin/bash

folder=submit03
prob_from=1
prob_to=213
#prob_from=299
#prob_to=299

for i in `seq $prob_from $prob_to`;
do
    format_i=`printf %03d $i`
    filepath=$folder/prob-${format_i}.sol
    text=`cat $filepath`
    curl -X POST -d "problem_id=${i}&author=futatsugi&solution=${text}" http://icfpc.logicmachine.jp:5000/submit
    echo $i
done
