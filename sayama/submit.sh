#!/bin/bash

folder=local-judge/answer/submit10
input_folder=../problems
prob_from=$1
mod=$2
prob_to=$3

for i in `seq $prob_from $mod $prob_to`;
do
    format_i=`printf %03d $i`
    filepath=${input_folder}/prob-${format_i}.desc
    text=`cat $filepath`
    boosterpath=${input_folder}/prob-${format_i}.buy
    booster=`cat $boosterpath`
    curl -X POST -d "problem_id=${i}&author=sayama3-clone&solution=${text}&boosters=${booster}" http://icfpc.logicmachine.jp:5000/submit
    echo $i $booster
done
