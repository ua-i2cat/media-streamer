#!/usr/bin/env bash

aclocal
autoconf
automake -a -v
cd dxt_compress; make; cd ..
./configure --prefix=$HOME/build
