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
* Bot disassembles filled voxel from leaf of BFS-tree.
* Bot fissions at most 40.
* Each bot erases each own leaf.

### sayama
* divide map by bfs and distribute to each cloned worker
* small empty cluster has high priority to travel
* if there is no neighbor empty cell, move to nearest empty cell by bfs.

### ikeda
* divide M in many squares.
* easy to paint square
* travel squares
