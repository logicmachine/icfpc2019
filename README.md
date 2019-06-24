# ICFP Programming Contest 2019

## Profile

### Team

fixstars

### Members

Noriyuki Futatsugi (futatsugi)
Takaaki Hiragushi (logicmachine)
Yuichi Sayama (sayama)
Atsuya Shibata (ashiba)
Tatsuya Ikeda (ikeda)

## Build

Compile and run it.

## Solution approach

### futatsugi
* Connnected component labelling algorithm to treat clustering of regions.
* Parallelization system using Intel TBB modules.
* Local search using the DFS for considering wrapping region.

### logicmachine/disassembler
* g++ -std=c++11 -Ofast main.cpp
* disassemble all using GVoid(28, 29, 30)

### ashiba
* Bot disassembles filled voxel from leaf of BFS-tree.
* Bot fissions at most 40.
* Each bot erases each own leaf.

### sayama
* create BFS tree and restrict nanobot’s motion as follows.
* The smaller the depth of BFS tree is, the earlier voxels are removed.
* The order of deletion is based on the dfs search of the BFS tree.

### ikeda
* calc dataset size ( x minimum index that we want to fill, y.. and maximum index )
* divide 5 x 8 cells
* send 40 bots on each section
* each bots fill cells from bot to top on their section
* if there is a section that ungrounded, my solver couldn’t solve
