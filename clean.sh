#!/bin/sh
set -e
. ./prepare-env.sh $1

for PROJECT in $PROJECTS; do
  (cd $PROJECT && $MAKE clean)
done

rm -rf isodir
rm -rf TalOS.iso
