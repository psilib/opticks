#include "G4VSolid.hh"

#include "GeoChain.hh"

#include <cstring>

#include "SSys.hh"
#include "SPath.hh"

#include "NNode.hpp"
#include "NCSG.hpp"

#include "GMesh.hh"
#include "GGeo.hh"
#include "X4PhysicalVolume.hh"

#include "CSGFoundry.h"
#include "CSG_GGeo_Convert.h"

#include "PLOG.hh"


const plog::Severity GeoChain::LEVEL = PLOG::EnvLevel("GeoChain", "DEBUG") ; 


#ifdef __APPLE__
const char* GeoChain::BASE = "$TMP/GeoChain_Darwin" ; 
#else
const char* GeoChain::BASE = "$TMP/GeoChain" ; 
#endif


GeoChain::GeoChain(Opticks* ok_)
    :
    ok(ok_), 
    ggeo(new GGeo(ok, true)),  // live=true to initLibs and not load from cache
    mesh(nullptr),
    volume(nullptr),
    fd(new CSGFoundry),
    lvIdx(0),  
    soIdx(0)  
{
    init();
}

void GeoChain::init()
{
    //for(int lvIdx=-1 ; lvIdx < 10 ; lvIdx+= 1 ) LOG(info) << " lvIdx " << lvIdx << " ok.isX4TubsNudgeSkip(lvIdx) " << ok->isX4TubsNudgeSkip(lvIdx)  ; 
}

void GeoChain::convertSolid(const G4VSolid* so, const std::string& meta_)
{
    LOG(info) << "[ meta " << meta_ ; 
    std::string lvname = so->GetName(); 

    mesh = X4PhysicalVolume::ConvertSolid(ok, lvIdx, soIdx, so, lvname ) ; 
    LOG(info) << " mesh " << mesh ; 
    convertMesh(mesh, meta_); 

    LOG(info) << "]" ;  
}


/**
GeoChain::convertNodeTree
---------------------------

TODO: "const nnode* nd" argument prevented by NCSG::Adopt,  
       maybe need to clone the node tree to support const argument  ?

TODO: factorise X4PhysicalVolume::ConvertSolid_ in order to 
      kick off the geochain with nnode and GMesh placeholder 
      without adding much code

**/

void GeoChain::convertNodeTree(nnode* raw, const std::string& meta_ )
{
    const G4VSolid* const solid = nullptr ; 
    bool balance_deep_tree = false ;  
    const char* soname = "convertNodeTree" ; 
    const char* lvname = "convertNodeTree" ; 

    mesh = X4PhysicalVolume::ConvertSolid_FromRawNode(ok, lvIdx, soIdx, solid, balance_deep_tree, raw, soname, lvname ); 

    convertMesh(mesh, meta_); 
}

void GeoChain::convertMesh(GMesh* mesh, const std::string& meta_ )
{
    const char* meta = meta_.empty() ? nullptr : meta_.c_str() ; 

    ggeo->add(mesh); 

    // standin for X4PhysicalVolume::convertStructure
    volume = X4PhysicalVolume::MakePlaceholderNode(); 
    ggeo->setRootVolume(volume);

    ggeo->prepareVolumes();   // creates GMergedMesh 

    ggeo->deferredCreateGParts(); 

    CSG_GGeo_Convert conv(fd, ggeo, meta ) ; 
    conv.convert();
}


/**
GeoChain::convertPV
---------------------

see okg4/tests/OKX4Test.cc as its doing much the same thing, maybe 
avoid duplication with some static methods 

**/

void GeoChain::convertPV( const G4VPhysicalVolume* top, const std::string& meta_ )
{
    const char* meta = meta_.empty() ? nullptr : meta_.c_str() ; 
    std::cout << "[ GeoChain::convertPV meta " << meta << std::endl ; 
    std::cout << "GeoChain::convertPV top " << top << std::endl ; 

    X4PhysicalVolume xtop(ggeo, top) ; 

    // ggeo->postDirectTranslation();  tries to save which fails with no idpath 
    // ggeo->prepareVolumes();   // just prepareVolumes  misses prepareOpticks which prevcents --skipsolidname from working 
    ggeo->prepare(); 

    ggeo->deferredCreateGParts();

    CSG_GGeo_Convert conv(fd, ggeo, meta ) ;  

    conv.convert();   // populates fd:CSGFoundry 

    std::cout 
        << "[ fd.descPrimSpec " << std::endl 
        << fd->descPrimSpec()
        << "] fd.descPrimSpec " << std::endl 
        << "[ fd.descMeshName " << std::endl 
        << fd->descMeshName()
        << "] fd.descMeshName " << std::endl 
        ; 


    std::cout << "] GeoChain::convert " << std::endl ; 
}

void GeoChain::save(const char* base, const char* name) const 
{
    int create_dirs = 2 ; // 2: dirpath
    const char* fold = SPath::Resolve(base, name, create_dirs );   
    const char* cfbase = SSys::getenvvar("CFBASE", fold  );
    const char* rel = "CSGFoundry" ; 

    fd->write(cfbase, rel );    // expects existing directory $CFBASE/CSGFoundry 

    CSGFoundry* lfd = CSGFoundry::Load(cfbase, rel);  // load foundary and check identical bytes
    assert( 0 == CSGFoundry::Compare(fd, lfd ) );  
}

