#!/bin/bash

for i in 0 1; do
    seq 0 3 > Player-Data/Input-P$i-0
done

function test
{
    if ! Scripts/$1.sh tutorial | grep 'weighted average: 2.333'; then
       Scripts/$1.sh tutorial
       exit 1
    fi
}

./compile.py -R 64 tutorial

for i in ring brain mal-rep-ring ps-rep-ring semi2k spdz2k; do
    test $i
done

./compile.py tutorial

for i in rep-field mal-rep-field ps-rep-field shamir mal-shamir hemi cowgear semi mascot; do
    test $i
done

./compile.py -B 16 tutorial

for i in replicated mal-rep-bin semi-bin yao tiny rep-bmr mal-rep-bmr shamir-bmr mal-shamir-bmr; do
    test $i
done
