#pragma once

#include <vector>
#include <string>
#include <glm/fwd.hpp>

#include <optix.h>

#include "plog/Severity.h"
#include "sframe.h"

#include "CSGOPTIX_API_EXPORT.hh"

struct SMeta ; 
struct NP ; 
struct quad4 ; 
struct quad6 ; 
struct qat4 ; 
struct float4 ; 

struct SGLM ; 
struct SSim ; 

struct CSGFoundry ; 
struct CSGView ; 

template <typename T> struct Tran ; 
struct QSim ; 
struct QEvent ; 

struct Params ; 
class Opticks ; 
class Composition ; 

#if OPTIX_VERSION < 70000
struct Six ; 
struct Dummy ; 
#else
struct Ctx ; 
struct PIP ; 
struct SBT ; 
#endif
struct Frame ; 

#include "SCSGOptiX.h"

struct CSGOPTIX_API CSGOptiX : public SCSGOptiX
{
    friend struct QSim ; 

    static const plog::Severity LEVEL ; 
    static CSGOptiX*   INSTANCE ; 
    static CSGOptiX*   Get(); 
    static int         Version(); 
    static const char* Desc(); 

    static const char* TOP ; 
    static const char* PTXNAME ; 
    static const char* GEO_PTXNAME ; 

#ifdef WITH_SGLM
#else
    Opticks*          ok ;  
    Composition*      composition ; 
#endif
    sframe            fr ;   // TODO: rehome to SEvt, can use pointer elsewhere
    SGLM*             sglm ; 

    const char*       moi ; 
    const char*       flight ; 
    const CSGFoundry* foundry ; 
    const char*       prefix ; 
    const char*       outdir ; 
    const char*       cmaketarget ; 
    const char*       ptxpath ; 
    const char*       geoptxpath ; 
    float             tmin_model ; 
    int               jpg_quality ; 

    std::vector<unsigned>  solid_selection ;
    std::vector<double>  launch_times ;

    int               raygenmode ; 
    Params*           params  ; 


#if OPTIX_VERSION < 70000
    Six* six ;  
    Dummy* dummy0 ; 
    Dummy* dummy1 ; 
#else
    Ctx* ctx ; 
    PIP* pip ; 
    SBT* sbt ; 
#endif

    Frame* frame ; 
    SMeta* meta ; 
    double dt ; 

    QSim*        sim ; 
    QEvent*      event ;  

    const char* desc() const ; 

private:
    static void InitGeo(  CSGFoundry* fd ); 
    static void InitSim( const SSim* ssim ); 
public:
    static CSGOptiX* Create(CSGFoundry* foundry ); 

    CSGOptiX(const CSGFoundry* foundry ); 

    void init(); 
    void initCheckSim(); 
    void initStack(); 
    void initParams();
    void initGeometry();
    void initRender();
    void initSimulate();

    static const char* Top() ; 
 private: 
    void setTop(const char* tspec); 
 public: 

    void setFrame(); 
    void setFrame(const char* moi);
    void setFrame(const float4& ce); 
    void setFrame(const sframe& fr); 


    void prepareRenderParam(); 
    void prepareSimulateParam(); 
    void prepareParam(); 

 private: 
    double launch(); 
 public: 
    void render_snap(const char* name=nullptr); 
 public: 
    // these methods fulfil SCSGOptix protocal base
    double render();   
 private: 
    // these methods get invoked from QSim::simtrace QSim::simulate following genstep uploading   
    double simtrace(); 
    double simulate();    
 public: 
    const CSGFoundry* getFoundry() const ; 
    static std::string Annotation( double dt, const char* bot_line, const char* extra=nullptr ); 
    const char* getDefaultSnapPath() const ; 
    void snap(const char* path=nullptr, const char* bottom_line=nullptr, const char* top_line=nullptr, unsigned line_height=24);  // part of SRenderer protocol base

    void writeFramePhoton(const char* dir, const char* name);
    int  render_flightpath(); 

    void saveMeta(const char* jpg_path) const ;


    static int   _OPTIX_VERSION() ; 
};

