/**
CXRaindropTest
======================

**/

#include <cuda_runtime.h>
#include <algorithm>
#include <csignal>
#include <iterator>

#include "scuda.h"
#include "sqat4.h"
#include "stran.h"
#include "NP.hh"

#include "SSys.hh"
#include "SCVD.hh"
#include "SPath.hh"
#include "SEventConfig.hh"
#include "SProc.hh"
#include "SOpticks.hh"
#include "SOpticksResource.hh"
#include "OpticksGenstep.h"

#include "OPTICKS_LOG.hh"
#include "Opticks.hh"

#include "CSGFoundry.h"
#include "CSGGenstep.h"
#include "CSGOptiX.h"
#include "RG.h"

#include "QBnd.hh"
#include "QSim.hh"
#include "QEvent.hh"
#include "SEvent.hh"

int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 
    SCVD::ConfigureVisibleDevices(); 

    Opticks ok(argc, argv ); 
    ok.configure(); 
    ok.setRaygenMode(RG_SIMULATE) ; // override --raygenmode option 

    const char* EXECUTABLE = SProc::ExecutableName();

    const char* cfbase = SOpticksResource::CFBase(); 
    const char* outfold = SEventConfig::OutFold();  
    LOG(info) << " outfold [" << outfold << "]"  ; 

    SOpticks::WriteOutputDirScript(outfold) ; // writes CSGOptiXSimulateTest_OUTPUT_DIR.sh in PWD 
    const char* idpath = SOpticksResource::IDPath();  // HMM: using OPTICKS_KEY geocache, not CSGFoundry
    const char* rindexpath = SPath::Resolve(idpath, "GScintillatorLib/LS_ori/RINDEX.npy", 0 );  

    // BASIS GEOMETRY : LOADED IN ORDER TO RAID ITS BOUNDARIES FOR MATERIALS,SURFACES

    CSGFoundry* fd = CSGFoundry::Load(cfbase, "CSGFoundry"); 
    if(fd->hasMeta()) LOG(info) << "fd.meta\n" << fd->meta ; 
    LOG(info) << fd->descComp(); 

    // fabricate some additional boundaries from the basis ones for this simple test 

    std::vector<std::string> specs = { "Rock/perfectAbsorbSurface/perfectAbsorbSurface/Air", "Air///Water" } ;  
    NP* optical_plus = nullptr ; 
    NP* bnd_plus = nullptr ; 
    QBnd::Add( &optical_plus, &bnd_plus, fd->optical, fd->bnd, specs );
    LOG(info) << std::endl << QBnd::DescOptical(optical_plus, bnd_plus)  ; 

    // qudarap does not depend on CSG so cannot consoldate (eg uploading "fd") here unless
    // rearrange to uploading a string, NP map : or do some protocal base gymnastics 

    QSim<float>::UploadComponents(fd->icdf, bnd_plus, optical_plus, rindexpath ); 

    const char* cfbase_local = SSys::getenvvar("CFBASE_LOCAL") ; assert(cfbase_local) ; 
    LOG(fatal) << "MIXING CSGFoundry combining basis cfbase with cfbase_local "; 
    std::cout << std::setw(20) << "cfbase" << ":" << cfbase << std::endl ; 
    std::cout << std::setw(20) << "cfbase_local" << ":" << cfbase_local  << std::endl ; 

    // load raindrop geometry and customize to use the boundaries added above 

    CSGFoundry* fdl = CSGFoundry::Load(cfbase_local, "CSGFoundry") ; 
    fdl->setOpticalBnd(optical_plus, bnd_plus);    // instanciates bd CSGName using bndplus.names
    fdl->saveOpticalBnd();                         // DIRTY: persisting new bnd and optical into the source directory : ONLY APPROPRIATE IN SMALL TESTS
    fdl->setPrimBoundary( 0, specs[0].c_str() ); 
    fdl->setPrimBoundary( 1, specs[1].c_str() );   // HMM: notice these boundary changes are not persisted 
    std::cout << "fdl.detailPrim " << std::endl << fdl->detailPrim() ; 
    fdl->upload(); 

    float4 ce = make_float4( 0.f, 0.f, 0.f, 100.f );   // TODO: this should come from the geometry 

    // HMM : WOULD BE BETTER FOR CONSISTENCY TO HAVE SINGLE UPLOAD API
    // must do this config before QEvent::init which happens with CSGOptiX instanciation 
    SEventConfig::SetMaxExtent( ce.w ); 
    SEventConfig::SetMaxTime( 10.f ); 


    // HMM: perhaps instanciate QEvent/QSim separately and give it as argument to CSGOptiX 

    CSGOptiX cx(&ok, fdl );   // QSim QEvent instanciated here 
    cx.setComposition(ce); 

    QEvent* event = cx.event ; 
    event->setGenstep(SEvent::MakeTorchGensteps());     

    cx.simulate();  
    cudaDeviceSynchronize(); 
    event->save(outfold); 

    return 0 ; 
}
