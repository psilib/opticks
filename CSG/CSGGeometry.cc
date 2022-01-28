#include "SSys.hh"
#include "SPath.hh"
#include "PLOG.hh"

#include "scuda.h"
#include "squad.h"
#include "SCenterExtentGenstep.hh"
#include "NP.hh"

#include "CSGQuery.h"
#include "CSGFoundry.h"
#include "CSGGeometry.h"
#include "CSGGrid.h"

CSGGeometry::CSGGeometry()
    :
    default_geom(nullptr),
    geom(SSys::getenvvar("GEOM", default_geom)),
    cfbase(SSys::getenvvar("CFBASE")), 
    name(nullptr),
    fd(nullptr),
    q(nullptr)
{
    LOG(info) << " GEOM " << geom  ; 
    init(); 
}

void  CSGGeometry::init()
{
    if( geom != nullptr && cfbase == nullptr)
    {
        init_geom(); 
    }
    else if(cfbase != nullptr)
    {
        init_cfbase();
    } 
    else
    {
        LOG(fatal) << " neither GEOM or CFBASE envvars are defined " ; 
        return ; 
    }
    q = new CSGQuery(fd); 
}

void  CSGGeometry::init_geom()
{
    name = strdup(geom); 
    LOG(info) << "init from GEOM " << geom << " name " << name ; 
    fd = CSGFoundry::Make(geom) ; 
}
void  CSGGeometry::init_cfbase()
{
    name = SPath::Basename(cfbase); 
    LOG(info) << "init from CFBASE " << cfbase << " name " << name  ; 
    fd = CSGFoundry::Load(cfbase, "CSGFoundry");
}

void CSGGeometry::saveSignedDistanceField() const 
{
    int resolution = SSys::getenvint("RESOLUTION", 25) ; 
    LOG(info) << " name " << name << " RESOLUTION " << resolution ; 
    q->dumpPrim();

    const CSGGrid* grid = q->scanPrim(resolution); 
    assert( grid );  
    grid->save(name);  
}

void CSGGeometry::saveCenterExtentGenstepIntersect() const 
{
    if(cfbase == nullptr || geom == nullptr)
    {
        LOG(fatal) << " require CFBASE and GEOM to control output dir " ; 
        return ;   
    }
    int create_dirs = 2 ;   
    const char* outdir = SPath::Resolve(cfbase, "CSGIntersectSolidTest", geom, create_dirs );  


    
    SCenterExtentGenstep* cegs = new SCenterExtentGenstep ; 

    const std::vector<quad4>& pp = cegs->pp ; 
    std::vector<quad4>& ii = cegs->ii ; 
    float t_min = 0.f ; 
    quad4 isect ;

    for(unsigned i=0 ; i < pp.size() ; i++)
    {   
        const quad4& p = pp[i]; 
        bool valid_intersect = q->intersect(isect, t_min, p );  
        if( valid_intersect )
        {   
            ii.push_back(isect);
        }   
    }   
    cegs->save(outdir); 
}

void CSGGeometry::intersectAgain( const char* path )
{
    if( path == nullptr ) path = "/tmp/s_isect.npy" ; 
    LOG(info) << " loading from " << path ; 
    NP* a = NP::Load(path); 
    if( a == nullptr ) 
    {
        LOG(fatal) << " no path " << path ; 
        return ; 
    }
    LOG(info) << " isect a " << a->sstr() ; 

    assert( a->shape.size() == 3 && a->shape[1] == 4 && a->shape[2] == 4 ); 

    std::vector<quad4> isects(a->shape[0]) ; 
    memcpy( isects.data(), a->bytes(),  sizeof(float)*16 );    
    const quad4& prev_isect = isects[0] ; 

    LOG(info) << " prev_isect.q1.f.x " << prev_isect.q1.f.x ;  
    
    quad4 isect ;
    bool valid_intersect = q->intersect_again(isect, prev_isect );  
    
    LOG(info) 
        << " valid_intersect " << valid_intersect 
        ;
}

