#!/usr/bin/env bash

. Scripts/run-common.sh

port=$[RANDOM+1024]

for i in 0 1; do
    IFS=""
    log="$*-$[1-i]"
    IFS=" "
    $prefix ./yao-party.x -p $i -pn $port $* 2>&1 | tee logs/$log & true
done

wait || exit 1
