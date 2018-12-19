#!/bin/bash

HERE=$(cd `dirname $0`; pwd)
SPDZROOT=$HERE/..

export PLAYERS=3

. $HERE/run-common.sh

run_player replicated-field-party.x ${1:-test_all} -lgp ${2:-128} || exit 1
