#!/bin/bash

folder=local-judge/answer/submit3
prob_from=299
prob_to=299

for i in `seq $prob_from $prob_to`;
do
    format_i=`printf %03d $i`
    filepath=$folder/prob-${format_i}.desc
    text=`cat $filepath`
    curl -X POST -d "problem_id=${i}&author=sayama&solution=${text}" http://icfpc.logicmachine.jp:5000/submit > /dev/null
done

