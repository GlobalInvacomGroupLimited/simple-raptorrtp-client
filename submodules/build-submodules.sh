#!/bin/bash

PREFIX=$PWD/../install


echo "Bulid Live555 "
SUBDIRS="groupsock liveMedia UsageEnvironment BasicUsageEnvironment"

( cd live555
  for subdir in ${SUBDIRS}; do
    echo $subdir
    echo "PREFIX = $PREFIX" >> $subdir/Makefile.head &&
    echo "LIBDIR = $PREFIX/lib" >> $subdir/Makefile.head ;
  done )

( cd live555 && echo "LIBDIR = $PREFIX/lib" >> Makefile.head && echo "PREFIX = $PREFIX" >> Makefile.head )

( cd live555 && ./genMakefiles linux-64bit )

( cd live555
  for subdir in ${SUBDIRS}; do 
    make -C $subdir;
  done )

( cd live555
  for subdir in ${SUBDIRS}; do 
    make -C $subdir install;
  done )


echo "Build raptor"
( cd raptor
  ./autogen.sh
  ./configure --enable-static --disable-shared --prefix=$PREFIX )

make -j 4 -C raptor

make install -C raptor


echo "Build raptorrtp"
( cd raptorrtp
  ./autogen.sh
  ./configure CPPFLAGS=-I$PREFIX/include \
              --with-liveMedia=$PREFIX/include/liveMedia \
              --with-BasicUsageEnvironment=$PREFIX/include/BasicUsageEnvironment \
              --with-groupsock=$PREFIX/include/groupsock \
              --with-UsageEnvironment=$PREFIX/include/UsageEnvironment \
              --prefix=$PREFIX \
              --enable-static --disable-shared --enable-logging )

make -j 4 -C raptorrtp

make install -C raptorrtp
