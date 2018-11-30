#!/bin/bash

HERE=$(cd `dirname $0`; pwd)
SPDZROOT=$HERE/..

$HERE/setup-ssl.sh 3

$SPDZROOT/Setup.x 3 128 0 online
