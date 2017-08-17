
// npy-
#include "NGLM.hpp"
#include "NPY.hpp"
#include "NSensor.hpp"
#include "Counts.hpp"
#include "Timer.hpp"


#include "GTreePresent.hh"
#include "GMergedMesh.hh"

//#include "GGeo.hh"

#include "GGeoLib.hh"
#include "GNodeLib.hh"

#include "GSolid.hh"
#include "GMatrix.hh"
#include "GBuffer.hh"
#include "GTreeCheck.hh"


#include "PLOG.hh"


// Following enabling of vertex de-duping (done at AssimpWrap level) 
// the below criteria are finding fewer repeats, for DYB only the hemi pmt
// TODO: retune

GTreeCheck::GTreeCheck(GGeoLib* geolib, GNodeLib* nodelib) 
       :
       m_geolib(geolib),
       m_nodelib(nodelib),
       m_repeat_min(120),
       m_vertex_min(300),  // aiming to include leaf? sStrut and sFasteners
       m_root(NULL),
       m_count(0),
       m_labels(0),
       m_digest_count(new Counts<unsigned>("progenyDigest"))
{
}

unsigned int GTreeCheck::getNumRepeats()
{
    return m_repeat_candidates.size();
}

void GTreeCheck::setRepeatMin(unsigned int repeat_min)
{
   m_repeat_min = repeat_min ; 
}
void GTreeCheck::setVertexMin(unsigned int vertex_min)
{
   m_vertex_min = vertex_min ; 
}

void GTreeCheck::createInstancedMergedMeshes(bool delta, unsigned verbosity)
{
    //assert(0);  
    Timer t("GTreeCheck::createInstancedMergedMeshes") ; 
    t.setVerbose(true);
    t.start();

    if(delta) 
    {
        deltacheck();
        t("deltacheck"); 
    }

    traverse();   // spin over tree counting up progenyDigests to find repeated geometry 
    t("traverse"); 

    labelTree();  // recursive setRepeatIndex on the GNode tree for each of the repeated bits of geometry
    t("labelTree"); 

    makeMergedMeshAndInstancedBuffers(verbosity);
    t("makeMergedMeshAndInstancedBuffers"); 


    t.stop();
    t.dump();
}





void GTreeCheck::traverse()
{
    m_root = m_nodelib->getSolid(0);
    assert(m_root);

    // count occurences of distinct progeny digests (relative sub-tree identities) in m_digest_count 
    traverse_r(m_root, 0);

    m_digest_count->sort(false);   // descending count order, ie most common subtrees first
    //m_digest_count->dump();

    // minrep 120 removes repeats from headonPMT, calibration sources and RPC leaving just PMTs 
   
    // collect digests of repeated pieces of geometry into  m_repeat_candidates
    findRepeatCandidates(m_repeat_min, m_vertex_min); 
    dumpRepeatCandidates();
}

void GTreeCheck::traverse_r( GNode* node, unsigned int depth)
{
    std::string& pdig = node->getProgenyDigest();
    m_digest_count->add(pdig.c_str());
    m_count++ ; 

    for(unsigned int i = 0; i < node->getNumChildren(); i++) traverse_r(node->getChild(i), depth + 1 );
}


void GTreeCheck::deltacheck()
{
    // check consistency of the level transforms
    m_root = m_nodelib->getSolid(0);
    assert(m_root);

    deltacheck_r(m_root, 0);
}

void GTreeCheck::deltacheck_r( GNode* node, unsigned int depth)
{
    GSolid* solid = dynamic_cast<GSolid*>(node) ;
    GMatrixF* gtransform = solid->getTransform();

    // solids levelTransform is set in AssimpGGeo and hails from the below with level -2
    //      aiMatrix4x4 AssimpNode::getLevelTransform(int level)
    //  looks to correspond to the placement of the LV within its PV  

    //GMatrixF* ltransform = solid->getLevelTransform();  
    GMatrixF* ctransform = solid->calculateTransform();
    float delta = gtransform->largestDiff(*ctransform);
    unsigned int nprogeny = node->getProgenyCount() ;

    if(nprogeny > 0 ) 
            LOG(debug) 
              << "GTreeCheck::deltacheck " 
              << " #progeny "  << std::setw(6) << nprogeny 
              << " delta*1e6 " << std::setprecision(6) << std::fixed << delta*1e6 
              << " name " << node->getName() 
              ;

    assert(delta < 1e-6) ;

    for(unsigned int i = 0; i < node->getNumChildren(); i++) deltacheck_r(node->getChild(i), depth + 1 );
}





struct GRepeat
{
    unsigned   repeat_min ; 
    unsigned   vertex_min ; 
    unsigned    index ; 
    std::string pdig ; 
    unsigned    ndig ; 
    GNode*      first ;   // cannot const as collection is deferred
    unsigned    nprog ; 
    unsigned    nvert ; 
    bool        candidate ; 
    bool        select ; 

    bool isListed(const std::vector<std::string>& pdigs_)
    {
        return std::find(pdigs_.begin(), pdigs_.end(), pdig ) != pdigs_.end() ;   
    }

    GRepeat( unsigned repeat_min_, unsigned vertex_min_, unsigned index_, const std::string& pdig_, unsigned ndig_, GNode* first_ ) 
          :
          repeat_min(repeat_min_),
          vertex_min(vertex_min_),
          index(index_),
          pdig(pdig_), 
          ndig(ndig_), 
          first(first_),
          nprog(first->getProgenyCount()),
          nvert(first->getProgenyNumVertices()),
          // includes self when GNode.m_selfdigest is true
          candidate(ndig > repeat_min && nvert > vertex_min ),
          select(false)
    {
    }

    std::string desc()
    { 
        std::stringstream ss ; 
        ss    << ( candidate ? " ** " : "    " ) 
              << ( select    ? " ## " : "    " ) 
              << " idx "   << std::setw(3) << index 
              << " pdig "  << std::setw(32) << pdig  
              << " ndig "  << std::setw(6) << ndig
              << " nprog " <<  std::setw(6) << nprog 
              << " nvert " <<  std::setw(6) << nvert
              << " n "     <<  first->getName() 
              ;  
        return ss.str();
    }

};



// suspect problem with allowing leaf repeaters is that digesta are not-specific enough, 
// so get bad matching 
//
//  allowing leaf repeaters results in too many, so place vertex count reqirement too 


void GTreeCheck::findRepeatCandidates(unsigned int repeat_min, unsigned int vertex_min)
{
    unsigned int nall = m_digest_count->size() ; 
    std::vector<GRepeat> cands ; 

    // over distinct subtrees (ie progeny digests)
    for(unsigned int i=0 ; i < nall ; i++)
    {
        std::pair<std::string,unsigned int>&  kv = m_digest_count->get(i) ;

        std::string& pdig = kv.first ; 
        unsigned int ndig = kv.second ;  // number of occurences of the progeny digest 

        GNode* first = m_root->findProgenyDigest(pdig) ; // first node that matches the progeny digest

        GRepeat cand(repeat_min, vertex_min,  i, pdig, ndig , first );
        cands.push_back(cand) ;

        if(cand.candidate) m_repeat_candidates.push_back(pdig);        
    }

    // erase repeats that are enclosed within other repeats 
    // ie that have an ancestor which is also a repeat candidate

    m_repeat_candidates.erase(
         std::remove_if(m_repeat_candidates.begin(), m_repeat_candidates.end(), *this ),
         m_repeat_candidates.end()
    ); 

    LOG(info) << "GTreeCheck::findRepeatCandidates"
              << " nall " << nall 
              << " repeat_min " << repeat_min 
              << " vertex_min " << vertex_min 
              << " cands " << cands.size()
              << " reps " << m_repeat_candidates.size()
              ;

    std::cout << " (**) candidates fulfil repeat/vert cuts   "  << std::endl ;
    std::cout << " (##) selected survive contained-repeat disqualification " << std::endl ;
 
    for(unsigned i=0 ; i < cands.size() ; i++)
    {
        GRepeat& cand = cands[i];
        cand.select = cand.isListed(m_repeat_candidates) ;
        std::cout << cand.desc() << std::endl; 
    }
    

}

bool GTreeCheck::operator()(const std::string& dig)  
{
    bool cr = isContainedRepeat(dig, 3);
 
    if(cr) LOG(info) 
                  << "GTreeCheck::operator() "
                  << " pdig "  << std::setw(32) << dig  
                  << " disallowd as isContainedRepeat "
                  ;

    return cr ;  
} 

bool GTreeCheck::isContainedRepeat( const std::string& pdig, unsigned int levels ) const 
{
    // for the first node that matches the *pdig* progeny digest
    // look back *levels* ancestors to see if any of the immediate ancestors 
    // are also repeat candidates, if they are then this is a contained repeat
    // and is thus disallowed in favor of the ancestor that contains it 

    GNode* node = m_root->findProgenyDigest(pdig) ; 
    std::vector<GNode*>& ancestors = node->getAncestors();  // ordered from root to parent 
    unsigned int asize = ancestors.size(); 

    for(unsigned int i=0 ; i < std::min(levels, asize) ; i++)
    {
        GNode* a = ancestors[asize - 1 - i] ; // from back to start with parent
        std::string& adig = a->getProgenyDigest();
        if(std::find(m_repeat_candidates.begin(), m_repeat_candidates.end(), adig ) != m_repeat_candidates.end())
        { 
            return true ;
        }
    }
    return false ; 
} 


void GTreeCheck::dumpRepeatCandidates()
{
    LOG(info) << "GTreeCheck::dumpRepeatCandidates " ;
    for(unsigned int i=0 ; i < m_repeat_candidates.size() ; i++) dumpRepeatCandidate(i) ;
}


void GTreeCheck::dumpRepeatCandidate(unsigned int index, bool verbose)
{
    std::string pdig = m_repeat_candidates[index];
    unsigned int ndig = m_digest_count->getCount(pdig.c_str());

    GNode* first = m_root->findProgenyDigest(pdig) ; // first node that matches the progeny digest
    std::vector<GNode*> placements = m_root->findAllProgenyDigest(pdig);
    std::cout  
                  << " pdig "  << std::setw(32) << pdig  
                  << " ndig "  << std::setw(6) << std::dec << ndig
                  << " nprog " <<  std::setw(6) << std::dec << first->getProgenyCount() 
                  << " placements " << std::setw(6) << placements.size()
                  << " n "          <<  first->getName() 
                  << std::endl 
                  ;  

    assert(placements.size() == ndig ); // restricting traverse to just selected causes this to fail
    if(verbose)
    {
        for(unsigned int i=0 ; i < placements.size() ; i++)
        {
            GNode* place = placements[i] ;
            GMatrix<float>* t = place->getTransform();
            std::cout 
                   << " t " << t->brief() 
                   << std::endl 
                   ;  
        }
    }
}

unsigned int GTreeCheck::getRepeatIndex(const std::string& pdig )
{
    // repeat index corresponding to a digest
     unsigned int index(0);
     std::vector<std::string>::iterator it = std::find(m_repeat_candidates.begin(), m_repeat_candidates.end(), pdig );
     if(it != m_repeat_candidates.end())
     {
         index = std::distance(m_repeat_candidates.begin(), it ) + 1;  // 1-based index
         LOG(debug)<<"GTreeCheck::getRepeatIndex " 
                  << std::setw(32) << pdig 
                  << " index " << index 
                  ;
     }
     return index ; 
}



void GTreeCheck::labelTree()
{
    m_labels = 0 ; 

    for(unsigned int i=0 ; i < m_repeat_candidates.size() ; i++)
    {
         std::string pdig = m_repeat_candidates[i];
         unsigned int ridx = getRepeatIndex(pdig);
         assert(ridx == i + 1 );
         std::vector<GNode*> placements = m_root->findAllProgenyDigest(pdig);

         // recursive labelling starting from the placements
         for(unsigned int p=0 ; p < placements.size() ; p++)
         {
             labelTree_r(placements[p], ridx);
         }
    }

    LOG(info)<<"GTreeCheck::labelTree count of non-zero setRepeatIndex " << m_labels ; 
}

void GTreeCheck::labelTree_r( GNode* node, unsigned int ridx)
{
    node->setRepeatIndex(ridx);
    if(ridx > 0)
    {
         LOG(debug)<<"GTreeCheck::labelTree "
                  << " ridx " << std::setw(5) << ridx
                  << " n " << node->getName()
                  ;
         m_labels++ ; 
    }
    for(unsigned int i = 0; i < node->getNumChildren(); i++) labelTree_r(node->getChild(i), ridx );
}


std::vector<GNode*> GTreeCheck::getPlacements(unsigned int ridx)
{
    std::vector<GNode*> placements ;
    if(ridx == 0)
    {
        placements.push_back(m_root);
    }
    else
    {
        assert(ridx >= 1); // ridx is a 1-based index
        assert(ridx-1 < m_repeat_candidates.size()); 
        std::string pdig = m_repeat_candidates[ridx-1];
        placements = m_root->findAllProgenyDigest(pdig);
        //placements = m_root->findAllProgenyDigest(pdig);
    } 
    return placements ; 
}

GNode* GTreeCheck::getRepeatExample(unsigned int ridx)
{
    std::vector<GNode*> placements = getPlacements(ridx);
    std::string pdig = m_repeat_candidates[ridx-1];
    GNode* node = m_root->findProgenyDigest(pdig) ; // first node that matches the progeny digest
    assert(placements[0] == node);
    return node ; 
}





void GTreeCheck::makeMergedMeshAndInstancedBuffers(unsigned verbosity)
{
    GNode* root = m_nodelib->getNode(0);
    assert(root); 
    GNode* base = NULL ; 


    // passes thru to GMergedMesh::create with management of the mm in GGeoLib
    GMergedMesh* mm0 = m_geolib->makeMergedMesh(0, base, root, verbosity );


    makeInstancedBuffers(mm0, 0);  // ? instanced global too, for common structure


    unsigned int numRepeats = getNumRepeats();
    for(unsigned int ridx=1 ; ridx <= numRepeats ; ridx++)  // 1-based index
    {
         GNode*   rbase  = getRepeatExample(ridx) ;    // <--- why not the parent ? off-by-one confusion here as to which transforms to include
         GMergedMesh* mm = m_geolib->makeMergedMesh(ridx, rbase, root, verbosity ); 

         makeInstancedBuffers(mm, ridx);
     
         //mm->reportMeshUsage( ggeo, "GTreeCheck::CreateInstancedMergedMeshes reportMeshUsage (instanced)");
    }
}

void GTreeCheck::makeInstancedBuffers(GMergedMesh* mergedmesh, unsigned int ridx)
{
     //mergedmesh->dumpSolids("GTreeCheck::makeInstancedBuffers dumpSolids");

     NPY<float>* itransforms = makeInstanceTransformsBuffer(ridx); // collect GNode placement transforms into buffer
     mergedmesh->setITransformsBuffer(itransforms);

     NPY<unsigned int>* iidentity  = makeInstanceIdentityBuffer(ridx);
     mergedmesh->setInstancedIdentityBuffer(iidentity);

     NPY<unsigned int>* aii   = makeAnalyticInstanceIdentityBuffer(ridx);
     mergedmesh->setAnalyticInstancedIdentityBuffer(aii);
}


NPY<float>* GTreeCheck::makeInstanceTransformsBuffer(unsigned int ridx)
{
    // collecting transforms from GNode instances into a buffer
    // getPlacement for ridx=0 just returns m_root (which always has identity transform)
    // for ridx > 0 returns all GNode instances 

    std::vector<GNode*> placements = getPlacements(ridx); 

    unsigned int ni = placements.size(); 
    if(ridx == 0)
        assert(ni == 1);

    NPY<float>* buf = NPY<float>::make(0, 4, 4);
    for(unsigned int i=0 ; i < ni ; i++)
    {
        GNode* place = placements[i] ;
        GMatrix<float>* t = place->getTransform();
        buf->add(t->getPointer(), 4*4*sizeof(float) );
    } 
    assert(buf->getNumItems() == ni);
    return buf ; 
}

NPY<unsigned int>* GTreeCheck::makeAnalyticInstanceIdentityBuffer(unsigned int ridx) 
{
    // collect identity information for each of the repeated nodes (or subtrees)
    // eg PMT sensor index

    std::vector<GNode*> placements = getPlacements(ridx);
    unsigned int numInstances = placements.size() ;
    if(ridx == 0)
        assert(numInstances == 1);


    NPY<unsigned int>* buf = NPY<unsigned int>::make(numInstances, 1, 4); // huh non-analytic uses (-1,4)
    buf->zero(); 

    // NB the differences:
    //
    //    analytic
    //         identity buffer has numInstances items (ie one entry for each repeated instance)
    //
    //    triangulated:  
    //         identity buffer has numInstances*numSolids items (ie one entry for every solid of every instance)
    //         ... downstream this gets repeated further to every triangle
    //


    unsigned int numProgeny = placements[0]->getProgenyCount();
    unsigned int numSolids  = numProgeny + 1 ; 

    // observe that each instance has only one sensor, so not need 
    // to repeat over the number of solids just one entry per instance

    LOG(info) << "GTreeCheck::makeAnalyticInstanceIdentityBuffer " 
              << " ridx " << ridx
              << " numPlacements " << numInstances
              << " numSolids " << numSolids      
              ;

    for(unsigned int i=0 ; i < numInstances ; i++) // over instances of the same geometry
    {
        GNode* base = placements[i] ;
        assert( numProgeny == base->getProgenyCount() );  // repeated geometry for the instances, so the progeny counts must match 
        std::vector<GNode*>& progeny = base->getProgeny();
        assert( progeny.size() == numProgeny );
      
        NSensor* sensor = NULL ;  
        for(unsigned int s=0 ; s < numSolids ; s++ )
        {
            GNode* node = s == 0 ? base : progeny[s-1] ; 
            GSolid* solid = dynamic_cast<GSolid*>(node) ;
            NSensor* ss = solid->getSensor();
            //assert(ss); dont have JUNO sensor info

            unsigned int sid = ss && ss->isCathode() ? ss->getId() : 0 ;

            if(sid > 0)
            LOG(debug) << "GTreeCheck::makeAnalyticInstanceIdentityBuffer " 
                      << " s " << std::setw(3) << s 
                      << " sid " << std::setw(10) << std::hex << sid << std::dec 
                      << " ss " << (ss ? ss->description() : "NULL" )
                      ;

            if(sid > 0 && ridx > 0)
            {
                assert(sensor == NULL && "not expecting more than one sensor solid with non-zero id within an instance of repeated geometry");
                sensor = ss ; 
            }
        }

        glm::uvec4 aii ; 

        aii.x = base->getIndex();        
        aii.y = i ;  // instance index (for triangulated this contains the mesh index)
        aii.z = 0 ;  // formerly boundary, but with analytic have broken 1-1 solid/boundary relationship so boundary must live in partBuffer
        aii.w = NSensor::RefIndex(sensor) ;  // the only critical one 

        buf->setQuadU(aii, i, 0); 
        
    }
    return buf ; 
}

NPY<unsigned int>* GTreeCheck::makeInstanceIdentityBuffer(unsigned int ridx) 
{
    /*
     Instances need to know the sensor they correspond 
     even though their geometry is duplicated. 

     For analytic geometry this is needed at the solid level 
     ie need buffer of size:
             #transforms * #solids-per-instance

     For triangulated geometry this is needed at the triangle level
     ie need buffer of size 
             #transforms * #triangles-per-instance

     The triangulated version can be created from the analytic one
     by duplication according to the number of triangles.

    */


    std::vector<GNode*> placements = getPlacements(ridx);
    unsigned int numInstances = placements.size() ;
    unsigned int numProgeny = placements[0]->getProgenyCount();
    unsigned int numSolids  = numProgeny + 1 ; 
    unsigned int num = numSolids*numInstances ; 

    NPY<unsigned int>* buf = NPY<unsigned int>::make(0, 4);
    for(unsigned int i=0 ; i < numInstances ; i++)
    {
        GNode* base = placements[i] ;
        assert( numProgeny == base->getProgenyCount() && "repeated geometry for the instances, so the progeny counts must match");
        std::vector<GNode*>& progeny = base->getProgeny();
        assert( progeny.size() == numProgeny );

        for(unsigned int s=0 ; s < numSolids ; s++ )
        {
            GNode* node = s == 0 ? base : progeny[s-1] ; 
            GSolid* solid = dynamic_cast<GSolid*>(node) ;

            guint4 id = solid->getIdentity();
            buf->add(id.x, id.y, id.z, id.w ); 

#ifdef DEBUG
            std::cout  
                  << " i " << i
                  << " s " << s
                  << " node/mesh/boundary/sensor " << id.x << "/" << id.y << "/" << id.z << "/" << id.w 
                  << " nodeName " << node->getName()
                  << std::endl 
                  ;
#endif
        }
    }
    assert(buf->getNumItems() == num);
    return buf ;  
}


/*

::

    In [1]: ii = np.load("iidentity.npy")

    In [3]: ii.shape
    Out[3]: (3360, 4)

    In [4]: ii.reshape(-1,5,4)
    Out[4]: 
    array([[[ 3199,    47,    19,     1],
            [ 3200,    46,    20,     2],
            [ 3201,    43,    21,     3],
            [ 3202,    44,     1,     4],
            [ 3203,    45,     1,     5]],

           [[ 3205,    47,    19,     6],
            [ 3206,    46,    20,     7],
            [ 3207,    43,    21,     8],
            [ 3208,    44,     1,     9],
            [ 3209,    45,     1,    10]],

After requiring an associated sensor surface to provide the sensor index, only cathodes 
have non-zero index::

    In [1]: ii = np.load("iidentity.npy")

    In [2]: ii.reshape(-1,5,4)
    Out[2]: 
    array([[[ 3199,    47,    19,     0],
            [ 3200,    46,    20,     0],
            [ 3201,    43,    21,     3],
            [ 3202,    44,     1,     0],
            [ 3203,    45,     1,     0]],

           [[ 3205,    47,    19,     0],
            [ 3206,    46,    20,     0],
            [ 3207,    43,    21,     8],
            [ 3208,    44,     1,     0],
            [ 3209,    45,     1,     0]],
*/


