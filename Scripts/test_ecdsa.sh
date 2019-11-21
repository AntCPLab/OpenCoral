#!/bin/bash

make -j4 ecdsa Fake-ECDSA.x

run()
{
    port=$[RANDOM+1024]
    if ! {
	    for j in $(seq 0 $2); do
		./$1-ecdsa-party.x -p $j 1 2>/dev/null & true
	    done
	    wait
	} | grep "Online checking"; then
	exit 1
    fi
}

for i in rep mal-rep shamir mal-shamir; do
    run $i 2
done

./Fake-ECDSA.x

for i in semi mascot fake-spdz; do
    run $i 1
done
