#!/bin/sh
set -e
. ./headers.sh $1

for PROJECT in $PROJECTS; do
  (cd $PROJECT && DESTDIR="$SYSROOT" $MAKE install)
done