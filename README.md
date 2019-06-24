# ICFP Programming Contest 2019

## Profile

### Team

* fixstars

### Members

* Noriyuki Futatsugi (futatsugi)
* Takaaki Hiragushi (logicmachine)
* Yuichi Sayama (sayama)
* Atsuya Shibata (ashiba)
* Tatsuya Ikeda (ikeda)

## Build

Compile and run it.

## Solution approach

### futatsugi
* Connnected component labelling algorithm to treat clustering of regions.
* Parallelization system using Intel TBB modules.
* Local search using the DFS for considering wrapping region.

### logicmachine/patch-optimizer
* g++ -std=c++17 -O3 main.cpp
* Replace from short sequence to more shorter sequence.

### ashiba
* g++ -std=c++11 -O2 main.cpp
* ./a.out left right
* ↑ Ecexute [left, right) testcase

### sayama
* divide map by bfs and distribute to each cloned worker
* small empty cluster has high priority to travel
* if there is no neighbor empty cell, move to nearest empty cell by bfs.

### ikeda
* divide M in many squares.
* easy to paint square
* travel squares
