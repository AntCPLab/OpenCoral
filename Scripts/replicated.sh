#!/bin/bash

for i in 0 1 2; do
    IFS=""
    log="replicated-$*-$i"
    IFS=" "
    $prefix ./replicated-bin-party.x -p $i $* | tee -a logs/$log & true
done

wait || exit 1
