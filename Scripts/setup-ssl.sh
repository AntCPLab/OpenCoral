#!/bin/bash

n=${1:-3}

echo Setting up SSL for $n parties

for i in `seq 0 $[n-1]`; do
    openssl req  -nodes -x509 -out Player-Data/P$i.pem -keyout Player-Data/P$i.key -subj "/CN=P$i"
done

c_rehash Player-Data
