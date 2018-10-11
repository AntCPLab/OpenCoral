#!/bin/bash

for i in 0 1; do
    IFS=""
    log="yao-$*-$i"
    IFS=" "
    $prefix ./yao-player.x -p $i $* | tee -a logs/$log & true
done

wait || exit 1
