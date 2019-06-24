#!/bin/bash

for i in {1..70} ; do
    zeroi=`printf %03d ${i}`
    fname="prob-"${zeroi}".desc"
    ans=`./Solver/run.sh ./desc/${fname}`
    echo $ans
    curl -X POST -d "problem_id=${i}&author=ikeda&solution=${ans}&boosters=" http://icfpc.logicmachine.jp:5000/submit
done

