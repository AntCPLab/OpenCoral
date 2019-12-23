#!/bin/bash

while getopts XC opt; do
    case $opt in
	X) compile_opts=-X
	   dabit=1
	   ;;
	C) cheap=1
	   ;;
    esac
done

shift $[OPTIND-1]

for i in 0 1; do
    seq 0 3 > Player-Data/Input-P$i-0
done

function test_vm
{
    ulimit -c unlimited
    if ! Scripts/$1.sh tutorial | grep 'weighted average: 2.333'; then
       Scripts/$1.sh tutorial
       exit 1
    fi
}

for dabit in ${dabit:-0 1}; do
    if [[ $dabit = 1 ]]; then
	compile_opts="$compile_opts -X"
    fi

    ./compile.py -R 64 $compile_opts tutorial

    for i in ring brain mal-rep-ring ps-rep-ring semi2k; do
	test_vm $i
    done

    if ! test "$dabit" = 1 -a "$cheap" = 1; then
	test_vm spdz2k
    fi

    ./compile.py  $compile_opts tutorial

    for i in rep-field mal-rep-field ps-rep-field; do
	test_vm $i
    done

    if [[ ! "$dabit" = 1 ]]; then
	for i in shamir mal-shamir; do
	    test_vm $i
	done
    fi

    for i in hemi semi; do
	test_vm $i
    done

    if ! test "$dabit" = 1 -a "$cheap" = 1; then
	for i in cowgear mascot; do
	    test_vm $i
	done
    fi
done

./compile.py -B 16  $compile_opts tutorial

for i in replicated mal-rep-bin semi-bin yao tinier tiny rep-bmr mal-rep-bmr shamir-bmr mal-shamir-bmr; do
    test_vm $i
done
