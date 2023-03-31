#!/bin/bash

(( $# < 2 )) && echo "Supply at least input file path and number of events. Output file prefix can ba passed as 3rd positional parameter. Event preselection can be switched off if 0 provided as 4th positional parameter" && exit 22

[ -z "$1" ] && echo "Provide input file name." && exit 22

set -o xtrace

DATAFILE=$1
NEVTS=$2

OUTPREF=""
(( $# > 2 )) && OUTPREF=$3

PRESELECT=1 ;
(( $# > 3 )) && PRESELECT=$4

INFNAME=$(basename $DATAFILE)
OUTFNAME=${INFNAME%.root}
[ -n "$OUTPREF" ] && OUTFNAME=$OUTPREF

WHEREAMI=$(dirname $0)

# create output dirs in case they don't exist
mkdir -p data logs

sed -e "s|larcv.root|data/${OUTFNAME}_larcv.root|" -e "s|SignalADCSpectrum.root|data/${OUTFNAME}_SignalADCSpectrum.root|" -e"s|DoPreselection: 1|DoPreselection: $PRESELECT|" ${WHEREAMI}/larcv_dune.fcl | lar -c - -n $NEVTS -s $DATAFILE -T data/${OUTFNAME}_hists.root --no-output &>logs/run_maker_${OUTFNAME}.log &
