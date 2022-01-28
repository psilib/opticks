#!/bin/bash -l 
msg="=== $BASH_SOURCE :"
usage(){ cat << EOU
csg_geochain.sh : CPU Opticks equivalent to cxs_geochain.sh with OptiX on GPU 
==========================================================================================

The idea behind this is to provide a convenient way to test code primarily intended to run on GPU 
in the more friendly debugging environment of the CPU.::

    IXIYIZ=4,0,0 ./csg_geochain.sh  ana
        selecting single genstep with IXIYIZ 

    IXIYIZ=5,0,0 ./csg_geochain.sh  ana

    IXIYIZ=0,6,0 ./csg_geochain.sh  ana      

EOU
}

#geom=AltXJfixtureConstruction_YZ
#geom=AltXJfixtureConstruction_XZ
#geom=AltXJfixtureConstruction_XY

#geom=AnnulusBoxUnion_XY
#geom=AnnulusTwoBoxUnion_XY
#geom=AnnulusOtherTwoBoxUnion_XY
#geom=AnnulusFourBoxUnion_XY
geom=AnnulusFourBoxUnion_YX

#geom=XJfixtureConstruction_YZ
#geom=XJfixtureConstruction_XZ
#geom=XJfixtureConstruction_XY


export GEOM=${GEOM:-$geom}

gcn=${GEOM%%_*}   ## name up to the first underscore, assuming use of axis suffix  _XZ _YZ _XY _ZX _ZY _YX 


if [ "$(uname)" == "Darwin" ] ; then
   cfbase=$TMP/GeoChain_Darwin/$gcn 
else
   cfbase=$TMP/GeoChain/$gcn 
fi 

dx=0
dy=0
dz=0
num_pho=100

case $gcn in 
  AnnulusFourBoxUnion) gridscale=0.1  ;; 
                    *) gridscale=0.15 ;;
esac


_AnnulusFourBoxUnion_YX(){
   note="couple of spurious intersects with IXIYIZ=5,0,0 and 0,6,0  " 
   ixiyiz=0,6,0

   export AGAIN=/tmp/s_isect.npy 
}

case $GEOM in 
   AnnulusFourBoxUnion_YX) _AnnulusFourBoxUnion_YX ;;
                        *) note="" ;;
esac

case $GEOM in  
   *_XZ) cegs=16:0:9:$dx:$dy:$dz:$num_pho  ;;  
   *_YZ) cegs=0:16:9:$dx:$dy:$dz:$num_pho  ;;  
   *_XY) cegs=16:9:0:$dx:$dy:$dz:$num_pho  ;;  
   *_ZX) cegs=9:0:16:$dx:$dy:$dz:$num_pho  ;;  
   *_ZY) cegs=0:9:16:$dx:$dy:$dz:$num_pho  ;;  
   *_YX) cegs=9:16:0:$dx:$dy:$dz:$num_pho  ;;  
   *_XYZ) cegs=9:16:9:$dx:$dy:$dz:$num_pho ;;  
esac

export NOTE=$note 
export GRIDSCALE=${GRIDSCALE:-$gridscale}
export CEGS=${CEGS:-$cegs}
export CFBASE=${CFBASE:-$cfbase}
export CEGS=${CEGS:-$cegs}
export IXIYIZ=${IXIYIZ:-$ixiyiz}


export TOPLINE="GEOM=$GEOM ./csg_geochain.sh "
export BOTLINE="$note"
export THIRDLINE="CEGS=$CEGS"




check_cegs()
{
    local msg="=== $FUNCNAME :"
    IFS=: read -a cegs_arr <<< "$CEGS"
    local cegs_elem=${#cegs_arr[@]}

    case $cegs_elem in
       4) echo $msg 4 element CEGS $CEGS ;;
       7) echo $msg 7 element CEGS $CEGS ;;
       *) echo $msg ERROR UNEXPECTED $cegs_elem element CEGS $CEGS && return 1  ;;
    esac
    return 0 
}

check_cfbase()
{
    local msg="=== $FUNCNAME :"
    if [ -n "$cfbase" -a ! -d "$cfbase/CSGFoundry" ]; then

       echo $msg : ERROR cfbase $cfbase is defined signalling to use a non-standard CSGFoundry geometry 
       echo $msg : BUT no such CSGFoundry directory exists 
       echo $msg :
       echo $msg : Possibilities: 
       echo $msg :
       echo $msg : 1. you intended to use the standard geometry but the GEOM $GEOM envvar does not match any of the if branches 
       echo $msg : 2. you want to use a non-standard geometry but have not yet created it : do so as shown below
       echo $msg :
       echo $msg :    \"b7 \; cd ~/opticks/GeoChain\"  
       echo $msg :    \"gc \; GEOM=$GEOM ./run.sh\" 
       echo $msg :   
       return 1
    fi
    return 0 
}

check_cfbase_file()
{
    local msg="=== $FUNCNAME :"
    if [ -n "$cfbase" -a -d "$cfbase/CSGFoundry" -a ! -f "$cfbase/CSGFoundry/meshname.txt" ]; then
       echo $msg : ERROR cfbase $cfbase is defined and the directory exists but it misses expected files 
       return 1 
    fi 
    return 0 
}


dumpvars(){ for var in $* ; do printf "%20s : %s \n" $var "${!var}" ; done ; }

check_cegs        || exit 1 
check_cfbase      || exit 1 
check_cfbase_file || exit 1 

dumpvars GEOM CEGS GRIDSCALE TOPLINE BOTLINE CFBASE NOTE IXIYIZ 


bin=CSGIntersectSolidTest
script=tests/CSGIntersectSolidTest.py 

arg=${1:-run_ana}
if [ "${arg/run}" != "$arg" ]; then

    echo $msg running binary $bin
    $bin
    [ $? -ne 0 ] && echo $msg run error && exit 1

elif [ "${arg/dbg}" != "$arg" ]; then 

    echo $msg running binary $bin under debugger
    if [ "$(uname)" == "Darwin" ]; then
        lldb__ $bin
    else
        gdb $bin
    fi 

fi

[ -n "$AGAIN" ] && echo $msg early exit as AGAIN $AGAIN is defined && exit 0 


if [ "${arg/ana}" != "$arg" ]; then

    echo $msg running script $script
    ${IPYTHON:-ipython} --pdb -i $script
    [ $? -ne 0 ] && echo $msg script error && exit 2
fi



exit 0
