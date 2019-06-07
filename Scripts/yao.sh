#!/bin/bash

. Scripts/run-common.sh

for i in 0 1; do
    IFS=""
    log="yao-$*-$i"
    IFS=" "
    $prefix ./yao-party.x -p $i $* 2>&1 | tee -a logs/$log & true
done

wait || exit 1
