#!/bin/bash

PREFIX=$PWD/install

echo "Bulid raptorrtpClient "
  ./autogen.sh
  ./configure CPPFLAGS=-I$PREFIX/include \
              --with-liveMedia=$PREFIX/include/liveMedia \
              --with-BasicUsageEnvironment=$PREFIX/include/BasicUsageEnvironment \
              --with-groupsock=$PREFIX/include/groupsock \
              --with-UsageEnvironment=$PREFIX/include/UsageEnvironment \
              --prefix=$PREFIX
make -j 4
make install
