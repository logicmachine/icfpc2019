#!/bin/bash

output_basedir=local-judge/answer/submit10

prob_from=$1
mod=$2
prob_to=$3

if [ ! -e $output_basedir ]; then
    mkdir $output_basedir
fi

for i in `seq $prob_from $mod $prob_to`;
do
    format_i=`printf %03d $i`
    filename=prob-${format_i}
    filepath=../problems/$filename
    ./a.out ${filepath}.desc ${filepath}.buy > $output_basedir/${filename}.sol
    echo $i
done
