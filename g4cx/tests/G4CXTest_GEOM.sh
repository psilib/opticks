#!/bin/bash -l 
usage(){ cat << EOU
G4CXTest_GEOM.sh : Standalone optical only bi-simulation with G4CXApp::Main and current GEOM 
================================================================================================

Standalone optical Geant4 initialization is faster than embedded Geant4 + Opticks but still ~120s to voxelize. 

Workstation::

    ~/opticks/g4cx/tests/G4CXTest_GEOM.sh
    ~/opticks/g4cx/tests/G4CXTest_GEOM.sh dbg
    LOG=1 BP=C4CustomART::doIt ~/opticks/g4cx/tests/G4CXTest_GEOM.sh dbg  

Laptop::

    ~/opticks/g4cx/tests/G4CXTest_GEOM.sh grab 
    EYE=0,-400,0 ~/opticks/g4cx/tests/G4CXTest_GEOM.sh ana


Where possible its better to use pure Opticks simulation (no bi-simulation) 
booting from a persisted geometry during testing due to the ~2s initialization time, eg with::

    ~/opticks/CSGOptiX/cxs_min.sh


storch::generate is used for both GPU and CPU generation
---------------------------------------------------------

* On GPU the generation is invoked at highest level CSGOptiX7.cu:simulate
* On CPU for example the stack is below, using MOCK_CURAND::

  G4RunManager::BeamOn 
  ... 
  G4RunManager::GenerateEvent 
  G4CXApp::GeneratePrimaries
  U4VPrimaryGenerator::GeneratePrimaries
  SGenerate::GeneratePhotons
  storch::generate    

EOU
}

SDIR=$(cd $(dirname $BASH_SOURCE) && pwd)

bin=G4CXTest
script=$SDIR/G4CXTest_GEOM.py 

source $HOME/.opticks/GEOM/GEOM.sh   # set GEOM and associated envvars for finding geometry

[ -n "$CVD" ] && export CUDA_VISIBLE_DEVICES=$CVD


## OPTICKS_INTEGRATION_MODE configures GPU and/or CPU optical simulation
#oim=1  # GPU only 
#oim=2  # CPU only 
oim=3   # GPU and CPU 
export OPTICKS_INTEGRATION_MODE=${OIM:-$oim} 


version=3
VERSION=${VERSION:-$version}
export VERSION    ## used in SEvt output directory name ALL$VERSION


TMP=${TMP:-/tmp/$USER/opticks}
export BASE=$TMP/GEOM/$GEOM
export LOGBASE=$BASE/$bin/ALL$VERSION
export AFOLD=$LOGBASE/p001 
export BFOLD=$LOGBASE/n001 
#export BFOLD=$TMP/GEOM/$GEOM/CSGOptiXSMTest/ALL/p001  ## TMP OVERRIDE COMPARE A-WITH-A from CSGOptiXSMTest

mkdir -p $LOGBASE
cd $LOGBASE            ## logfile written in invoking directory 


#export G4CXOpticks__SaveGeometry_DIR=$BASE  ## optionally save geom into BASE for debug 
export G4CXApp__SensDet=PMTSDMgr             ## used for post GDML SensDet hookup


case $VERSION in 
 0) oem=Minimal ;;
 1) oem=HitOnly ;; 
 2) oem=HitAndPhoton ;; 
 3) oem=HitAndPhoton ;;    ## USING 3 FOR LEAK TEST 
99) oem=StandardFullDebug ;;
esac 

## OPTICKS_EVENT_MODE configures the SEvt components to gather and save
## for now are tieing that with VERSION 

export OPTICKS_EVENT_MODE=${OEM:-$oem}   
export OPTICKS_MAX_BOUNCE=31
export OPTICKS_MAX_PHOTON=M100  ## sstr::ParseScale h/K/H/M prefixes 
export OPTICKS_NUM_EVENT=19     ## MUST MATCH num elem in OPTICKS_NUM_PHOTON below


#srm=SRM_DEFAULT
srm=SRM_TORCH
#srm=SRM_INPUT_PHOTON
#srm=SRM_INPUT_GENSTEP    ## NOT IMPLEMENTED FOR GEANT4
#srm=SRM_GUN
export OPTICKS_RUNNING_MODE=$srm


echo $BASH_SOURCE OPTICKS_RUNNING_MODE $OPTICKS_RUNNING_MODE

if [ "$OPTICKS_RUNNING_MODE" == "SRM_TORCH" ]; then 
    #export SEvent_MakeGenstep_num_ph=$NUM   ## trumped by OPTICKS_NUM_PHOTON

    #onp=K1:10 
    #onp=H1:10,M2,3,5,7,10,20,40,80,100
    #onp=M3,10   
    #onp=M10
    #onp=M1,2,3
    onp=H1:10
    ## NB NEEDS TO BE WITHIN MAX_PHOTON constaint 

    export OPTICKS_NUM_PHOTON=${ONP:-$onp}

    #src="rectangle"
    #src="disc"
    src="sphere"

    if [ "$src" == "rectangle" ]; then
        export storch_FillGenstep_pos=0,0,0
        export storch_FillGenstep_type=rectangle
        export storch_FillGenstep_zenith=-20,20
        export storch_FillGenstep_azimuth=-20,20
    elif [ "$src" == "disc" ]; then
        export storch_FillGenstep_type=disc
        export storch_FillGenstep_radius=50      
        export storch_FillGenstep_zenith=0,1       # radial range scale
        export storch_FillGenstep_azimuth=0,1      # phi segment twopi fraction 
        export storch_FillGenstep_mom=1,0,0
        export storch_FillGenstep_pos=-80,0,0
    elif [ "$src" == "sphere" ]; then
        export storch_FillGenstep_type=sphere
        export storch_FillGenstep_radius=100    # +ve for outwards    
        export storch_FillGenstep_pos=0,0,0
        export storch_FillGenstep_distance=1.00 # frac_twopi control of polarization phase(tangent direction)
    fi 

elif [ "$OPTICKS_RUNNING_MODE" == "SRM_INPUT_PHOTON" ]; then 
    echo -n 

elif [ "$OPTICKS_RUNNING_MODE" == "SRM_INPUT_GENSTEP" ]; then 
    echo -n 

elif [ "$OPTICKS_RUNNING_MODE" == "SRM_GUN" ]; then 
    echo -n 
fi 

logging()
{
   export SSim=INFO
   export QSim=INFO
   export QPMT=INFO
   #export SEvt=INFO
   #export U4Recorder=INFO
   #export U4StepPoint=INFO
   export U4Physics=INFO
   #export CSGFoundry=INFO
   #export CSGTarget=INFO

   #export U4GDML__VERBOSE=1 
   #export SPMTAccessor__VERBOSE=1
   export SEvt__LIFECYCLE=1  ## sparse SEvt debug output, works well alone  
}
[ -n "$LOG" ] && logging
[ -n "$LIFECYCLE" ] && export SEvt__LIFECYCLE=1

defarg="info_env_run_report_ana"
#defarg="info_dbg_ana"
arg=${1:-$defarg}

vars="BASH_SOURCE SDIR GEOM ${GEOM}_CFBaseFromGEOM ${GEOM}_GDMLPath VERSION TMP BASE LOGBASE AFOLD BFOLD CVD CUDA_VISIBLE_DEVICES script" 


if [ "${arg/info}" != "$arg" ]; then 
    for var in $vars ; do printf "%50s : %s \n" "$var" "${!var}" ; done 
fi 

if [ "${arg/env}" != "$arg" ]; then 
    env | grep OPTICKS | perl -n -e 'm/(\S*)=(\S*)/ && printf("%50s : %s\n", $1, $2) ' -
fi 

if [ "${arg/run}" != "$arg" ]; then
    rm -f $bin.log
    $bin
    [ $? -ne 0 ] && echo $BASH_SOURCE : run error && exit 1 
fi 

if [ "${arg/dbg}" != "$arg" ]; then
    dbg__ $bin
    [ $? -ne 0 ] && echo $BASH_SOURCE : dbg error && exit 2 
fi 

if [ "${arg/report}" != "$arg" ]; then
    sreport
    [ $? -ne 0 ] && echo $BASH_SOURCE : sreport error && exit 1 
fi 

if [ "${arg/plot}" != "$arg" ]; then
    runprof=1 $SDIR/../../sysrap/tests/sreport.sh ana
    [ $? -ne 0 ] && echo $BASH_SOURCE : sreport.plot error && exit 1 
fi 

if [ "${arg/grab}" != "$arg" ]; then
    source $SDIR/../../bin/rsync.sh $LOGBASE    ## widen to BASE to include the debug geometry save 
    [ $? -ne 0 ] && echo $BASH_SOURCE : grab error && exit 3 
fi

if [ "${arg/ana}" != "$arg" ]; then
    ${IPYTHON:-ipython} --pdb -i $script 
    [ $? -ne 0 ] && echo $BASH_SOURCE : ana error with script $script && exit 4
fi 

exit 0 

