
usage(){ cat << EOU
FewPMT.sh ( formerly hamaLogicalPMT.sh ) 
===========================================

This geomscript is sourced from::

   U4SimtraceTest.sh
   U4SimulateTest.sh


EOU
}


version=${1:-0}
pom=${POM:-0}

#layout=two_pmt  
layout=one_pmt
export LAYOUT=$layout

## moved LAYOUT control inside geomscript so its in common 
## between U4SimulateTest.sh and U4SimtraceTest.sh 


case $layout in 
  one_pmt) echo layout $layout ;; 
  two_pmt) echo layout $layout ;; 
esac

case $version in
  0) echo $BASH_SOURCE N=0 unnatural geometry : FastSim/jPOM ;;
  1) echo $BASH_SOURCE N=1 natural geometry : CustomBoundary ;;
esac

case $pom in 
   0) echo POM $pom : traditional stop at photocathode : PMT with no innards  ;;
   1) echo POM $pom : allow photons into PMT which has innards ;; 
esac

fastcover=Cheese


## PMTFastSim/HamamatsuR12860PMTManager declProp config 

export hama_FastCoverMaterial=$fastcover
export hama_UsePMTOpticalModel=$pom     
export hama_UseNaturalGeometry=$version 

export nnvt_FastCoverMaterial=$fastcover
export nnvt_UsePMTOpticalModel=$pom   
export nnvt_UseNaturalGeometry=$version 


geomlist=hamaLogicalPMT,nnvtLogicalPMT     # in one_pmt layout get NNVT with this 
#geomlist=nnvtLogicalPMT,hamaLogicalPMT    # in one_pmt layout get HAMA with this
#geomlist=nnvtLogicalPMT
#geomlist=hamaLogicalPMT

export ${GEOM}_GEOMList=$geomlist

aspect=1.7777777777777  # 1280/720

echo $BASH_SOURCE layout $layout 

if [ "$layout" == "one_pmt" ]; then 

   export U4VolumeMaker_WrapRockWater_Rock_HALFSIDE=210
   export U4VolumeMaker_WrapRockWater_Water_HALFSIDE=200
   export U4VolumeMaker_WrapRockWater_BOXSCALE=$aspect,1,1

elif [ "$layout" == "two_pmt" ]; then 

    export U4VolumeMaker_WrapAroundItem_Rock_HALFSIDE=310  
    export U4VolumeMaker_WrapAroundItem_Water_HALFSIDE=300  
    export U4VolumeMaker_WrapAroundItem_Rock_BOXSCALE=$aspect,1,1
    export U4VolumeMaker_WrapAroundItem_Water_BOXSCALE=$aspect,1,1 

    export ${GEOM}_GEOMWrap=AroundCircle 

    export U4VolumeMaker_MakeTransforms_AroundCircle_radius=250
    export U4VolumeMaker_MakeTransforms_AroundCircle_numInRing=2
    export U4VolumeMaker_MakeTransforms_AroundCircle_fracPhase=0

else
    echo $BASH_SOURCE layout $layout not handled 
fi 


# Simtrace config
export CEGS=16:0:9:10   


