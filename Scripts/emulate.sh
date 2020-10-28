#!/bin/bash

test -e logs || mkdir logs
prog=${1%.sch}
prog=${prog##*/}
shift
./emulate.x $prog $* 2>&1 | tee -a logs/emulate-$prog
