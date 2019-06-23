#!/bin/bash

folder=submit03
prob_from=1
prob_to=300
#prob_from=11
#prob_to=12

for i in `seq $prob_from $prob_to`;
do
    format_i=`printf %03d $i`
    srcpath=../../problems/prob-${format_i}.desc
    filepath=$folder/prob-${format_i}.sol
    #text=`cat $filepath`
    #curl -X POST -d "problem_id=${i}&author=futatsugi&solution=${text}" http://icfpc.logicmachine.jp:5000/submit
    ./solver_nf $srcpath > $filepath
    echo $i
done
