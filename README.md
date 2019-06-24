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
* create BFS tree and restrict nanobot’s motion as follows.
* The smaller the depth of BFS tree is, the earlier voxels are removed.
* The order of deletion is based on the dfs search of the BFS tree.

### ikeda
* divide M in many squares.
* easy to paint square
* travel squares
