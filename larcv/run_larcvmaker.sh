#!/bin/bash

OUTPREF=""
PRESELECT=1 ;

usage() {
  echo "Usage: $0 [-pP] [-n NEVENTS] [-o OUTPUTPREFIX] INFNAME" 1>&2
}
exit_abnormal() {                         # Function: Exit with error.
  usage
  exit 1
}

(( $# < 1 )) && usage && exit 22

NEVTS=1

while getopts ":pPn:o:" options; do         # Loop: Get the next option;
                                          # use silent error checking;
                                          # option n takes an argument.
  case "${options}" in                    #
    n)                                    # If the option is n,
      NEVTS=${OPTARG}                      # set $NAME to specified value.
      ;;
    p)
	PRESELECT=1
	;;
    P)
	PRESELECT=0
	;;
    o)
	OUTPREF=${OPTARG}
	;;
    :)                                    # If expected argument omitted:
      echo "Error: -${OPTARG} requires an argument."
      exit_abnormal                       # Exit abnormally.
      ;;
    *)                                    # If unknown (any other) option:
      exit_abnormal                       # Exit abnormally.
      ;;
  esac
done

DATAFILE="${@:$OPTIND:1}"
[ -z "$DATAFILE" ] && echo "Provide input file name." && exit 22

set -o xtrace

INFNAME=$(basename $DATAFILE)
OUTFNAME=${INFNAME%.root}
[ -n "$OUTPREF" ] && OUTFNAME=$OUTPREF


WHEREAMI=$(dirname $0)

# create output dirs in case they don't exist
mkdir -p data logs

sed -e "s|larcv.root|data/${OUTFNAME}_larcv.root|" -e "s|SignalADCSpectrum.root|data/${OUTFNAME}_SignalADCSpectrum.root|" -e"s|DoPreselection: 1|DoPreselection: $PRESELECT|" ${WHEREAMI}/larcv_dune.fcl | lar -c - -n $NEVTS -s $DATAFILE -T data/${OUTFNAME}_hists.root --no-output &>logs/run_maker_${OUTFNAME}.log &
