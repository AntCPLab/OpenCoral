#!/bin/bash

n=${1:-3}

test -e Player-Data || mkdir Player-Data

echo Setting up SSL for $n parties

for i in `seq 0 $[n-1]`; do
    openssl req -newkey rsa -nodes -x509 -out Player-Data/P$i.pem -keyout Player-Data/P$i.key -subj "/CN=P$i"
done

# brew-installed OpenSSL on MacOS
PATH=$PATH:/usr/local/opt/openssl/bin/
c_rehash Player-Data
