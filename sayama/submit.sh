#!/bin/bash

folder=local-judge/answer/submit9
prob_to=300
prob_from=$1
mod=$2

for i in `seq $prob_from $mod $prob_to`;
do
    format_i=`printf %03d $i`
    filepath=$folder/prob-${format_i}.desc
    text=`cat $filepath`
    curl -X POST -d "problem_id=${i}&author=sayama3&solution=${text}" http://icfpc.logicmachine.jp:5000/submit
    echo $i
done
