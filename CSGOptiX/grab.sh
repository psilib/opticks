#!/bin/bash -l 

grab_usage(){ cat << EOU
grab.sh
=============

Runs rsync between a remote geocache/CSG_GGeo/ directory into which cxs 
intersect "photon" arrays are persisted and local directories. 
The remote directory to grab is configurable with envvar OPTICKS_KEYDIR_GRABBED,  eg::

   .opticks/geocache/DetSim0Svc_pWorld_g4live/g4ok_gltf/3dbec4dc3bdef47884fe48af781a179d/1

NB to update the CSGFoundry geometry on laptop for correct labelling of volumes use::

   ./cf_grab.sh 


EOU
}

arg=${1:-all}
shift

executable=${EXECUTABLE:-CSGOptiXSimulateTest}
default_opticks_keydir_grabbed=.opticks/geocache/DetSim0Svc_pWorld_g4live/g4ok_gltf/3dbec4dc3bdef47884fe48af781a179d/1
opticks_keydir_grabbed=${OPTICKS_KEYDIR_GRABBED:-$default_opticks_keydir_grabbed}

xdir=$opticks_keydir_grabbed/CSG_GGeo/$executable/   ## trailing slash to avoid duplicating path element 

from=P:$xdir
to=$HOME/$xdir

LOGDIR=/tmp/$USER/opticks/CSGOptiX/$EXECUTABLE


printf "arg                    %s \n" "$arg"
printf "EXECUTABLE             %s \n " "$EXECUTABLE"
printf "LOGDIR                 %s \n " "$LOGDIR"
printf "OPTICKS_KEYDIR_GRABBED %s \n " "$OPTICKS_KEYDIR_GRABBED" 
printf "opticks_keydir_grabbed %s \n " "$opticks_keydir_grabbed" 
printf "\n"
printf "xdir                   %s \n" "$xdir"
printf "from                   %s \n" "$from" 
printf "to                     %s \n" "$to" 

mkdir -p $to


open_last(){
    local msg="=== $FUNCNAME :"
    local typ=${1:-jpg}
    local last=$(ls -1rt `find ${to%/} -name "*.$typ" ` | tail -1 )
    echo $msg typ $typ last $last
    if [ "$(uname)" == "Darwin" ]; then
        open $last 
    fi 
}

if [ "$arg" == "tab" ]; then

    globptn="${to}cvd1/70000/cxr_overview/cam_0_tmin_0.4/cxr_overview*.jpg"
    refjpgpfx="/env/presentation/cxr/cxr_overview"

    ${IPYTHON:-ipython} -i $(which snap.py) --  --globptn "$globptn" --refjpgpfx "$refjpgpfx" $*

elif [ "$arg" == "tab_water" ]; then

    globptn="${to}cvd1/70000/sWaterTube/cxr_view/*/cxr_view_sWaterTube.jpg"
    ${IPYTHON:-ipython} -i $(which snap.py) --  --globptn "$globptn"  $*

elif [ "$arg" == "png" ]; then

    rsync -zarv --progress --include="*/" --include="*.png" --exclude="*" "$from" "$to"
    ls -1rt `find ${to%/} -name '*.png'`
    open_last png

elif [ "$arg" == "jpg" ]; then

    rsync -zarv --progress --include="*/" --include="*.jpg" --include="*.json" --exclude="*" "$from" "$to"
    ls -1rt `find ${to%/} -name '*.jpg' -o -name '*.json'`
    open_last jpg

elif [ "$arg" == "mp4" ]; then

    rsync -zarv --progress --include="*/" --include="*.mp4" --include="*.json" --exclude="*" "$from" "$to"
    ls -1rt `find ${to%/} -name '*.mp4' -o -name '*.json'`
    open_last mp4


elif [ "$arg" == "all" ]; then
    rsync -zarv --progress --include="*/" --include="*.txt" --include="*.npy" --include="*.jpg" --include="*.mp4" --include "*.json" --exclude="*" "$from" "$to"

    ls -1rt `find ${to%/} -name '*.json' -o -name '*.txt' `
    ls -1rt `find ${to%/} -name '*.jpg' -o -name '*.mp4' -o -name '*.npy'  `


    if [ "$EXECUTABLE" == "CSGOptiXSimulateTest" -o "$EXECUTABLE" == "CSGOptiXRenderTest" ]; then  

        last_npy=$(ls -1rt `find ${to%/} -name '*.npy' ` | tail -1 )
        last_outdir=$(dirname $last_npy)
        last_outbase=$(dirname $last_outdir)
        last_outleaf=$(basename $last_outdir)

        ## write source-able script ${EXECUTABLE}_OUTPUT_DIR.sh defining correponding envvar
        ## depending on the path of the last .npy grabbed  
        ## This is used from cxs.sh to transparently communicate the last OUTPUT_DIR 
        ## between nodes. 

        script=$LOGDIR/${EXECUTABLE}_OUTPUT_DIR.sh
        mkdir -p $(dirname $script)

        echo last_npy $last_npy 
        echo last_outdir $last_outdir 
        echo last_outbase $last_outbase
        echo last_outleaf $last_outleaf

        echo script $script
        printf "export ${EXECUTABLE}_OUTPUT_DIR=$last_outdir\n" > $script
        cat $script
   fi 

fi 

