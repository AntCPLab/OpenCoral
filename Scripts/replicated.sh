#!/bin/bash

port=$[RANDOM+1024]

for i in 0 1 2; do
    IFS=""
    log="replicated-$*-$i"
    IFS=" "
    $prefix ./replicated-bin-party.x -p $i -pn $port $* 2>&1 |
	{
	    if test $i = 0; then
		tee -a logs/$log
	    else
		cat >> logs/$log
	    fi
	} & true
done

wait || exit 1
