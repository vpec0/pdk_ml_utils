#!/bin/bash
set -o xtrace

DEST=$PWD
PWD=$(dirname $0)

cd $PWD
[ -f "convert" ] && [ "convert" -nt "convert.C" ] && exit 0

LARCV_FLAGS="-I$LARCV_INCDIR -L$LARCV_LIBDIR -lLArCVCoreDataFormat"
g++ -O3 convert.C $(root-config --glibs --cflags --libs) $LARCV_FLAGS -o convert

cp convert $DEST/
