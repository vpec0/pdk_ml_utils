# source this file and provide dunesw version and qualifiers,
# e.g.
#   source setup.sh v09_66_02d00 e20:prof
#
# And provide LARCV_BASEDIR environment variable pointing to LArCV installation

VERSION=$1
QUALS=$2

[ -z "$LARCV_BASEDIR" ] && echo "Set LARCV_BASDIR env." && return

source /cvmfs/dune.opensciencegrid.org/products/dune/setup_dune.sh
setup dunesw $VERSION -q $QUALS
. ../dev/larcvmaker/localProducts_larsoft_${VERSION%???}_${QUALS/:/_}/setup
mrbslp

export LARCV_INCDIR=$LARCV_BASEDIR/include
export LARCV_LIBDIR=$LARCV_BASEDIR/lib

export LD_LIBRARY_PATH=$LARCV_LIBDIR:$LD_LIBRARY_PATH
