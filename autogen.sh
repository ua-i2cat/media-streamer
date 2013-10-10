#!/usr/bin/env bash

libtoolize
aclocal
autoconf
automake -a -v
cd dxt_compress; make; cd ..
./configure $@
