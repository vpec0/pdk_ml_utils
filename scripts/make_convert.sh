#!/bin/bash
set -o xtrace

DEST=$PWD
PWD=$(dirname $0)

TARGET=convert
[ -n "$1" ] && TARGET=$1

cd $PWD
[ -f "$TARGET" ] && [ "$TARGET" -nt "$TARGET.C" ] && exit 0

LARCV_FLAGS="-I$LARCV_INCDIR -L$LARCV_LIBDIR -lLArCVCoreDataFormat"
g++ -O3 $TARGET.C $(root-config --glibs --cflags --libs) $LARCV_FLAGS -o $TARGET

[ $PWD != $DEST ] && cp $TARGET $DEST/
