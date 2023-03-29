#!/bin/bash

#echo $#

FNAME=test #logs/run_maker_protondecay_hA_BodekRitchie_dune10kt_1x2x6_54474279_179_20220423T063923Z_gen_g4_detsim_reco_65804491_0_20230126T175412Z_reReco.log
(( $# > 1 )) && FNAME=$2

EVENT=0
(( $# > 0 )) && EVENT=$1

# echo $1 $2
# echo $EVENT

cat $FNAME  | grep -e "p ->" -e "K ->" | nl -v0 -bp'p ->' | grep -A1 -E "^[[:space:]]+$EVENT[[:space:]]" | grep -E -e "^[[:space:]]+$EVENT[[:space:]]" -e"K ->"
#TYPESTRING=$(cat $FNAME  | grep -A1 -e "p ->" -e "K ->" | head -$[EVENT*5 + 4] | nl -v0 -bp'p ->'| tail -4) # | tr '\n' ';')

#echo $TYPESTRING

# CORENAME=$(basename $FNAME)
# CORENAME=${CORENAME/run_maker_/""}
# CORENAME=${CORENAME/%.log/""}
# HISTFNAME=$(ls data/*${CORENAME}*"_2dhists.root")
# root -l "drawHistogram.C(\"$HISTFNAME\", $EVENT, \"\")"
