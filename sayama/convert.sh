#!/bin/bash

for file in `ls ../problems/*.desc`; do
    filename=`basename $file`
    output="../ascii-problems/$filename"
    ./to_ascii $file $output
done
