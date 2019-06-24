#!/bin/sh

#cd futatsugi/sayama
c++ solver_nf_parallel.cpp -O3 -std=c++14 -ltbb -o solver_nf_parallel
