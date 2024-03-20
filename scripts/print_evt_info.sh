#!/bin/bash

#echo $#

usage() {
  echo "Usage: $0 [ -n NEVENT ] LOGFNAME" 1>&2
}
exit_abnormal() {                         # Function: Exit with error.
  usage
  exit 1
}

EVENT=-1

while getopts ":n:" options; do         # Loop: Get the next option;
                                          # use silent error checking;
                                          # option n takes an argument.
  case "${options}" in                    #
    n)                                    # If the option is n,
      EVENT=${OPTARG}                      # set $NAME to specified value.
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

FNAME=test #logs/run_maker_protondecay_hA_BodekRitchie_dune10kt_1x2x6_54474279_179_20220423T063923Z_gen_g4_detsim_reco_65804491_0_20230126T175412Z_reReco.log
FNAME="${@:$OPTIND:1}"
[ -z "$FNAME" ] && echo "Supply input file name." 1>&2 && exit_abnormal
[ ! -f $FNAME ] && echo "File $FNAME is not accessible." 1>&2 && exit_abnormal

# echo $1 $2
# echo $EVENT

if [ $EVENT -eq -1 ] ; then
    cat $FNAME  | grep -e " BackTracker" -e "p ->" -e "K ->" | nl -v0 -bp'p ->'
else
    cat $FNAME  | grep -e " BackTracker" -e "p ->" -e "K ->" | nl -v0 -bp'p ->' | grep -B1 -A1 -E "^[[:space:]]+$EVENT[[:space:]]" | grep -B1 -E -e "^[[:space:]]+$EVENT[[:space:]]" -e"K ->" |sed -e"s/.*run: \(.*\) sub.* event: /Run \1 Event /"
fi
#TYPESTRING=$(cat $FNAME  | grep -A1 -e "p ->" -e "K ->" | head -$[EVENT*5 + 4] | nl -v0 -bp'p ->'| tail -4) # | tr '\n' ';')

#echo $TYPESTRING

# CORENAME=$(basename $FNAME)
# CORENAME=${CORENAME/run_maker_/""}
# CORENAME=${CORENAME/%.log/""}
# HISTFNAME=$(ls data/*${CORENAME}*"_2dhists.root")
# root -l "drawHistogram.C(\"$HISTFNAME\", $EVENT, \"\")"
