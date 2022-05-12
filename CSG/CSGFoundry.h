#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "plog/Severity.h"

struct SBitSet ; 
struct NP ; 

#include "scuda.h"
#include "squad.h"
#include "sqat4.h"
#include "saabb.h"
#include "stran.h"


struct CSGName ; 
struct CSGTarget ; 
struct CSGMaker ; 
struct CSGGenstep ; 

#include "CSGEnum.h"
#include "CSGSolid.h"
#include "CSGPrim.h"
#include "CSGNode.h"


#include "CSG_API_EXPORT.hh"

/**
CSGFoundry
============

::

        +-------+     +-------+    +---------+     +--------+       +-------+   +-------+        +------+
        | inst  |     | solid |    |  prim   |     | node   |       | tran  |   | itra  |        | plan |
        +=======+     +=======+    +=========+     +========+       +=======+   +=======+        +======+
        | s=0   | ->  |       | -> |         | ->  |        |  ->   |       |   |       |        |      |
        | s=0   |     |       |    |         |     |        |       |       |   |       |        |      |
        | s=0   |     |       |    +---------+     |        |       |       |   |       |        |      |
        | s=0   |     +-------+    |         |     |        |       |       |   |       |        +------+
        | s=1   |                  |         |     |        |       |       |   |       |
        | s=1   |                  |         |     +--------+       |       |   |       |
        | s=1   |                  |         |     |        |       |       |   |       |
        | s=1   |                  +---------+     |        |       |       |   |       |
        +-------+                                  |        |       |       |   |       |
                                                   |        |       |       |   |       |
                                                   |        |       |       |   |       |
                                                   |        |       |       |   |       |
                                                   |        |       |       |   |       |
                                                   |        |       |       |   |       |
                                                   +--------+       |       |   |       |  
                                                                    |       |   |       |
                                                                    |       |   |       |
                                                                    |       |   |       |
                                                                    |       |   |       |
                                                                    +-------+   +-------+
       
inst 
   array of qat4 with single solid references (gas_idx)

solid
   references range of prim with (numPrim, primOffset)  
prim
   references range of node with (numNode, nodeOffset) (roughly corresponds to G4VSolid)
node
   references single transform with tranOffset 
   sometimes references range of planes with (planOffset, numPlan)
   (CSG constituent nodes : sphere, box, boolean operators)

tran
    array of quat4 node transforms, same size as itra
itra
    array of quat4 node inverse transforms, same size as tran
plan 
    array of float4 planes 


**/


struct CSG_API CSGFoundry
{
    static const plog::Severity LEVEL ; 
    static const int  VERBOSE ; 
    static const unsigned IMAX ; 
    static const char* BASE ; 
    static const char* RELDIR ; 


    static CSGFoundry* MakeGeom(const char* geom);
    static CSGFoundry* LoadGeom(const char* geom=nullptr); 

    static CSGFoundry* MakeDemo(); 
    static CSGFoundry* Load();
    static CSGFoundry* Load_();
    static CSGFoundry* Load(const char* base, const char* rel);
    //static CSGFoundry* Load(const char* dir );

    static int Compare(const CSGFoundry* a , const CSGFoundry* b ); 

    template<typename T>
    static int CompareVec( const char* name, const std::vector<T>& a, const std::vector<T>& b );

    static int CompareBytes(const void* a, const void* b, unsigned num_bytes);


    CSGFoundry();
    void init(); 

    const char* getFold() const ;
    void setFold(const char* fold); 
    void setGeom(const char* geom); 
    void setOrigin(const CSGFoundry* origin); 
    void setElv(const SBitSet* elv); 

    std::string desc() const ;
    std::string descComp() const ;
    std::string descSolid() const ; 
    std::string descMeshName() const ; 
    std::string descGAS() const ;

    void summary(const char* msg="CSGFoundry::summary") const ;
    std::string descSolids() const ;
    std::string descInst(unsigned ias_idx_, unsigned long long emm=~0ull ) const ;

    void dump() const ;
    void dumpSolid() const ;
    void dumpSolid(unsigned solidIdx ) const ;
    int findSolidIdx(const char* label) const  ; // -1 if not found
    void findSolidIdx(std::vector<unsigned>& solid_idx, const char* label) const ; 
    std::string descSolidIdx( const std::vector<unsigned>& solid_selection ) ; 

    void dumpPrim() const ;
    void dumpPrim(unsigned solidIdx ) const ;
    std::string descPrim() const ;
    std::string descPrim(unsigned solidIdx) const  ;

    int getPrimBoundary(unsigned primIdx) const ; 
    void setPrimBoundary(unsigned primIdx, const char* bname) ; 
    void setPrimBoundary(unsigned primIdx, unsigned boundary) ; 

    std::string detailPrim() const ; 
    std::string detailPrim(unsigned primIdx) const ; 

    std::string descPrimSpec() const ; 
    std::string descPrimSpec(unsigned solidIdx) const ; 


    void dumpNode() const ;
    void dumpNode(unsigned solidIdx ) const ;
    std::string descNode() const ;
    std::string descNode(unsigned solidIdx) const  ;
    std::string descTran(unsigned solidIdx) const  ; 


    AABB iasBB(unsigned ias_idx_, unsigned long long emm=0ull ) const ;
    float4 iasCE(unsigned ias_idx_, unsigned long long emm=0ull ) const;
    void   iasCE(float4& ce, unsigned ias_idx_, unsigned long long emm=0ull ) const;
    void   gasCE(float4& ce, unsigned gas_idx ) const ;
    void   gasCE(float4& ce, const std::vector<unsigned>& gas_idxs ) const ; 

    float  getMaxExtent(const std::vector<unsigned>& solid_selection) const ;
    std::string descSolids(const std::vector<unsigned>& solid_selection) const ;


    CSGPrimSpec getPrimSpec(       unsigned solidIdx) const ;
    CSGPrimSpec getPrimSpecHost(   unsigned solidIdx) const ;
    CSGPrimSpec getPrimSpecDevice( unsigned solidIdx) const ;
    void        checkPrimSpec(     unsigned solidIdx) const ;
    void        checkPrimSpec() const ;


    const CSGSolid*   getSolidByName(const char* name) const ;
    const CSGSolid*   getSolid_(int solidIdx) const ;   // -ve counts from back 
    unsigned          getSolidIdx(const CSGSolid* so) const ; 


    unsigned getNumSolid(int type_) const ;
    unsigned getNumSolid() const;        // STANDARD_SOLID count 
    unsigned getNumSolidTotal() const;   // all solid count 

    unsigned getNumPrim() const;   
    unsigned getNumNode() const;
    unsigned getNumPlan() const;
    unsigned getNumTran() const;
    unsigned getNumItra() const;
    unsigned getNumInst() const;

    const CSGSolid*   getSolid(unsigned solidIdx) const ;  
    const CSGPrim*    getPrim(unsigned primIdx) const ;    
    const CSGNode*    getNode(unsigned nodeIdx) const ;
    const float4*     getPlan(unsigned planIdx) const ;
    const qat4*       getTran(unsigned tranIdx) const ;
    const qat4*       getItra(unsigned itraIdx) const ;
    const qat4*       getInst(unsigned instIdx) const ;

    CSGNode*          getNode_(unsigned nodeIdx);


    void              getNodePlanes(std::vector<float4>& planes, const CSGNode* nd) const ;  
    const CSGPrim*    getSolidPrim(unsigned solidIdx, unsigned primIdxRel) const ;
    const CSGNode*    getSolidPrimNode(unsigned solidIdx, unsigned primIdxRel, unsigned nodeIdxRel) const ;

    void getMeshPrimCopies(  std::vector<CSGPrim>& select_prim, unsigned mesh_idx ) const ;
    void getMeshPrimPointers(std::vector<const CSGPrim*>& select_prim, unsigned mesh_idx ) const ; 
    const CSGPrim* getMeshPrim( unsigned midx, unsigned mord ) const ; 

    unsigned getNumMeshPrim(unsigned mesh_idx ) const ;
    std::string descMeshPrim() const ;  

    unsigned getNumSelectedPrimInSolid(const CSGSolid* solid, const SBitSet* elv ) const ; 


    CSGSolid* addSolid(unsigned num_prim, const char* label, int primOffset_ = -1 );
    CSGPrim*  addPrim(int num_node, int nodeOffset_ ) ;   // former defaults meshIdx:-1, nodeOffset_:-1

    CSGNode*  addNode(CSGNode nd, const std::vector<float4>* pl=nullptr, const Tran<double>* tr=nullptr );
    CSGNode*  addNodes(const std::vector<CSGNode>& nds );

    CSGNode*  addNode(AABB& bb, CSGNode nd );
    CSGNode*  addNodes(AABB& bb, std::vector<CSGNode>& nds, const std::vector<const Tran<double>*>* trs  ); 

    float4*   addPlan(const float4& pl );


    CSGSolid* addDeepCopySolid(unsigned solidIdx, const char* label=nullptr );


    template<typename T> unsigned addTran( const Tran<T>* tr  );
    template<typename T> unsigned addTran_( const Tran<T>* tr  );
    unsigned addTran( const qat4* tr, const qat4* it ) ;
    unsigned addTran() ;
    void     addTranPlaceholder(); 

    // adds transform and associates it with the node
    template<typename T> const qat4* addNodeTran(CSGNode* nd, const Tran<T>* tr, bool transform_node_aabb  ); 
    void addNodeTran(CSGNode* nd ); 


    void     addInstance(const float* tr16, unsigned gas_idx, unsigned ias_idx ); 
    void     addInstancePlaceholder(); 


    void makeDemoSolids();   // via maker
    CSGSolid* make(const char* name); 


    static void DumpAABB(const char* msg, const float* aabb); 


    const char* getBaseDir(bool create) const ; 

    void write(const char* dir) const ;
    void write(const char* base, const char* rel) const ;
    void saveOpticalBnd() const ; 


    // these argumentless methods require CFBASE envvar or geom member to be set 
    void write() const ; 
    void load() ; 

    void load( const char* base, const char* rel ) ; 
    void setCFBase( const char* cfbase_ ); 
    const char* getCFBase() const ; 
    const char* getOriginCFBase() const ; 


    void load( const char* dir ) ; 

    template<typename T> void loadArray( std::vector<T>& vec, const char* dir, const char* name, bool optional=false ); 

    void upload();
    void inst_find_unique(); 

    unsigned getNumUniqueIAS() const ;
    unsigned getNumUniqueGAS() const ;
    unsigned getNumUniqueINS() const ;

    unsigned getNumInstancesIAS(unsigned ias_idx, unsigned long long emm) const ;
    void     getInstanceTransformsIAS(std::vector<qat4>& select_inst, unsigned ias_idx, unsigned long long emm ) const ;

    unsigned getNumInstancesGAS(unsigned gas_idx) const ;
    void     getInstanceTransformsGAS(std::vector<qat4>&  select_qv, unsigned gas_idx ) const ;  // collecting by value : TODO eliminate, swiching to getInstancePointersGAS
    void     getInstancePointersGAS(  std::vector<const qat4*>& select_qi, unsigned gas_idx ) const ;  // collecting pointers to the actual instances 

    const qat4* getInstanceGAS(unsigned gas_idx_ , unsigned ordinal=0) const  ;


    // target  
    int getCenterExtent(float4& ce, int midx, int mord, int iidx=-1, qat4* m2w=nullptr, qat4* w2m=nullptr ) const ;
    int getTransform(   qat4& q   , int midx, int mord, int iidx=-1) const ;

    // id 
    void parseMOI(int& midx, int& mord, int& iidx, const char* moi) const ; 
    const char* getName(unsigned midx) const ;  

    template <typename T> void setMeta( const char* key, T value ); 
    template <typename T> T    getMeta( const char* key, T fallback); 
    bool hasMeta() const ; 

    void kludgeScalePrimBBox( const char* label, float dscale );
    void kludgeScalePrimBBox( unsigned solidIdx, float dscale );

    unsigned getNumMeshName() const ; 
    unsigned getNumSolidLabel() const ; 

    static void CopyNames(    CSGFoundry* dst, const CSGFoundry* src ); 
    static void CopyMeshName( CSGFoundry* dst, const CSGFoundry* src ); 

    void getMeshName( std::vector<std::string>& mname ) const ; 

    const std::string& getMeshName(unsigned midx) const ; 
    const std::string& getBndName(unsigned bidx) const ; 
    const std::string descELV(const SBitSet* elv); 

    const std::string& getSolidLabel(unsigned sidx) const ; 

    void addMeshName(const char* name); 
    void addSolidLabel(const char* label); 

    void setOpticalBnd( const NP* optical, const NP* bnd ); // "foreigner" ? TODO:SSim


    std::vector<std::string> meshname ;  // GGeo::getMeshNames/GMeshLib (G4VSolid names from Geant4) should be primName in CF model ?
    std::vector<std::string> mmlabel ;   // from GGeo::getMergedMeshLabels eg of form "3084:sWorld" "7:HamamatsuR12860sMask_virtual"

    std::vector<CSGSolid>  solid ;   
    std::vector<CSGPrim>   prim ; 
    std::vector<CSGNode>   node ; 
    std::vector<float4>    plan ; 
    std::vector<qat4>      tran ;  
    std::vector<qat4>      itra ;  
    std::vector<qat4>      inst ;  

    CSGPrim*    d_prim ; 
    CSGNode*    d_node ; 
    float4*     d_plan ; 
    qat4*       d_itra ; 

    std::vector<unsigned>  ins ; 
    std::vector<unsigned>  gas ; 
    std::vector<unsigned>  ias ; 

    CSGName*    id ;   // meshname 

    CSGTarget*  target ; 
    CSGGenstep* genstep ; 
    CSGMaker*   maker ; 
    bool        deepcopy_everynode_transform ; 

    CSGSolid*   last_added_solid ; 
    CSGPrim*    last_added_prim ; 
    CSGNode*    last_added_node ; 


    // holding these "foreign" QSim input arrays directly here feels wrong, 
    // better to manage in SSim wrapper struct singleton to avoid playing pass the parcel
    const NP*    optical ; 
    const NP*        bnd ; 
    const CSGName*    bd ;  // instanciated by setOpticalBnd using bnd->names
    NP* icdf ;              // scintillation icdf  



    std::string meta ; 
    const char* fold ; 
    const char* cfbase ; 
    const char* geom ; 
    const char* loaddir ; 

    const CSGFoundry* origin ; 
    const SBitSet*    elv ; 

};


