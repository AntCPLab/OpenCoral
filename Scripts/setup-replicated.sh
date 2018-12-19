#!/bin/bash

HERE=$(cd `dirname $0`; pwd)
SPDZROOT=$HERE/..

# prime field bit length
bits=${1:-128}

$HERE/setup-ssl.sh 3

$SPDZROOT/Setup.x 3 ${bits} 0 online
