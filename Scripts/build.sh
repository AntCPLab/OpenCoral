#!/bin/bash

function build
{
    echo ARCH = $1 >> CONFIG.mine
    echo GDEBUG = >> CONFIG.mine
    make clean
    rm -R static
    mkdir static
    make -j 12 static-hm
    mkdir bin
    dest=bin/`uname`-$2
    rm -R $dest
    mv static $dest
    strip $dest/*
}

build '' amd64
build '-msse4.1 -maes -mpclmul' aes
build '-msse4.1 -maes -mpclmul -mavx -mavx2 -mbmi2' avx2
