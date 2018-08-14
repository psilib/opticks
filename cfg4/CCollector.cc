#include <sstream>

#include "NLookup.hpp"
#include "NPY.hpp"
#include "CCollector.hh"

#include "OpticksHub.hh"
#include "OpticksPhoton.h"

#include "PLOG.hh"

CCollector* CCollector::INSTANCE = NULL ;

CCollector* CCollector::Instance()
{
   assert(INSTANCE && "CCollector has not been instanciated");
   return INSTANCE ;
}

CCollector::CCollector(const NLookup* lookup)    
    :
    m_lookup(lookup),
    m_genstep(NPY<float>::make(0,6,4)),
    m_genstep_itemsize(m_genstep->getNumValues(1)),
    m_genstep_values(new float[m_genstep_itemsize]),
    m_scintillation_count(0),
    m_cerenkov_count(0),
    m_machinery_count(0),
    m_primary(NPY<float>::make(0,4,4)),
    m_primary_itemsize(m_primary->getNumValues(1)),
    m_primary_values(new float[m_primary_itemsize]),
    m_primary_count(0)
{
    assert( m_genstep_itemsize == 6*4 );
    assert( m_primary_itemsize == 4*4 );
    assert( m_lookup && m_lookup->isClosed() ); 
    INSTANCE = this ; 
}

int CCollector::translate(int acode) const // raw G4 materialId translated into GBndLib material line for GPU usage 
{
    int bcode = m_lookup->a2b(acode) ;
    return bcode ; 
}

NPY<float>*  CCollector::getGensteps() const 
{
    consistencyCheck() ;
    return m_genstep ; 
}

void CCollector::setGensteps(NPY<float>* gs)
{
    m_genstep = gs ; 
}

NPY<float>*  CCollector::getPrimary() const 
{
    return m_primary ; 
}


void CCollector::consistencyCheck() const 
{
     unsigned numItems = m_genstep->getNumItems();
     bool consistent = numItems == m_scintillation_count + m_cerenkov_count + m_machinery_count ;
     if(!consistent)
         LOG(fatal) << "CCollector::consistencyCheck FAIL " 
                    << description()
                    ;
     assert(consistent);
}

std::string CCollector::description() const
{
    std::stringstream ss ; 
    ss << " CCollector "
       << " numItems " << m_genstep->getNumItems() 
       << " scintillation_count " << m_scintillation_count
       << " cerenkov_count " << m_cerenkov_count
       << " machinery_count " << m_machinery_count
       << " step_count " << m_scintillation_count + m_cerenkov_count + m_machinery_count 
       << " primary_count " << m_primary_count
       ;
    return ss.str();
}

void CCollector::Summary(const char* msg) const 
{ 
    LOG(info) << msg 
              << description()
              ;
}


void CCollector::collectScintillationStep
(
            G4int                /*id*/, 
            G4int                parentId,
            G4int                materialId,
            G4int                numPhotons,
            
            G4double             x0_x,  
            G4double             x0_y,  
            G4double             x0_z,  
            G4double             t0, 

            G4double             deltaPosition_x, 
            G4double             deltaPosition_y, 
            G4double             deltaPosition_z, 
            G4double             stepLength, 

            G4int                pdgCode, 
            G4double             pdgCharge, 
            G4double             weight, 
            G4double             meanVelocity, 

            G4int                scntId,      
            G4double             slowerRatio,
            G4double             slowTimeConstant,
            G4double             slowerTimeConstant,

            G4double             scintillationTime,
            G4double             scintillationIntegralMax,
            G4double             spare1,
            G4double             spare2
) 
{
     m_scintillation_count += 1 ;   // 1-based index
     LOG(debug) 
          << " scintillation_count " << m_scintillation_count ;

     uif_t uifa[4] ;
     uifa[0].i = SCINTILLATION ; 

    // id == 0 ? m_scintillation_count : id  ;   // use the 1-based index when id zero 
     uifa[1].i = parentId ; 
     uifa[2].i = translate(materialId) ;   // raw G4 materialId translated into GBndLib material line for GPU usage 
     uifa[3].i = numPhotons ; 

     uif_t uifb[4] ;
     uifb[0].i = pdgCode ;
     uifb[1].i = scntId ;
     uifb[2].i = 0 ;
     uifb[3].i = 0 ;

     //////////// 6*4 floats for one step ///////////

     float* ss = m_genstep_values ; 

     ss[0*4+0] = uifa[0].f ;
     ss[0*4+1] = uifa[1].f ;
     ss[0*4+2] = uifa[2].f ;
     ss[0*4+3] = uifa[3].f ;

     ss[1*4+0] = x0_x ;
     ss[1*4+1] = x0_y ;
     ss[1*4+2] = x0_z ;
     ss[1*4+3] = t0 ;

     ss[2*4+0] = deltaPosition_x ;
     ss[2*4+1] = deltaPosition_y ;
     ss[2*4+2] = deltaPosition_z ;
     ss[2*4+3] = stepLength ;

     ss[3*4+0] = uifb[0].f ;  // pdgCode
     ss[3*4+1] = pdgCharge ;
     ss[3*4+2] = weight ;
     ss[3*4+3] = meanVelocity ;

     ss[4*4+0] = uifb[1].f ;  // scntId
     ss[4*4+1] = slowerRatio ;
     ss[4*4+2] = slowTimeConstant ;
     ss[4*4+3] = slowerTimeConstant ;

     ss[5*4+0] = scintillationTime ;
     ss[5*4+1] = scintillationIntegralMax ;
     ss[5*4+2] = spare1 ;
     ss[5*4+3] = spare2 ;

     m_genstep->add(ss, m_genstep_itemsize);
}



void CCollector::collectCerenkovStep
(
            G4int              /*id*/, 
            G4int                parentId,
            G4int                materialId,
            G4int                numPhotons,
            
            G4double             x0_x,  
            G4double             x0_y,  
            G4double             x0_z,  
            G4double             t0, 

            G4double             deltaPosition_x, 
            G4double             deltaPosition_y, 
            G4double             deltaPosition_z, 
            G4double             stepLength, 

            G4int                pdgCode, 
            G4double             pdgCharge, 
            G4double             weight, 
            G4double             meanVelocity, 

            G4double             betaInverse,
            G4double             pmin,
            G4double             pmax,
            G4double             maxCos,

            G4double             maxSin2,
            G4double             meanNumberOfPhotons1,
            G4double             meanNumberOfPhotons2,
            G4double             spare2
)
{
     m_cerenkov_count += 1 ;   // 1-based index
     LOG(debug) 
           << " cerenkov_count " << m_cerenkov_count ;

     uif_t uifa[4] ;
     uifa[0].i = CERENKOV ; 
   // id == 0 ? -m_cerenkov_count : id  ;   // use the negated 1-based index when id zero 
     uifa[1].i = parentId ; 
     uifa[2].i = translate(materialId) ; 
     uifa[3].i = numPhotons ; 

     uif_t uifb[4] ;
     uifb[0].i = pdgCode ;
     uifb[1].i = 0 ;
     uifb[2].i = 0 ;
     uifb[3].i = 0 ;

     //////////// 6*4 floats for one step ///////////

     float* cs = m_genstep_values ; 

     cs[0*4+0] = uifa[0].f ;
     cs[0*4+1] = uifa[1].f ;
     cs[0*4+2] = uifa[2].f ;
     cs[0*4+3] = uifa[3].f ;

     cs[1*4+0] = x0_x ;
     cs[1*4+1] = x0_y ;
     cs[1*4+2] = x0_z ;
     cs[1*4+3] = t0 ;

     cs[2*4+0] = deltaPosition_x ;
     cs[2*4+1] = deltaPosition_y ;
     cs[2*4+2] = deltaPosition_z ;
     cs[2*4+3] = stepLength ;

     cs[3*4+0] = uifb[0].f ;  // pdgCode
     cs[3*4+1] = pdgCharge ;
     cs[3*4+2] = weight ;
     cs[3*4+3] = meanVelocity ;

     cs[4*4+0] = betaInverse ;  
     cs[4*4+1] = pmin ;
     cs[4*4+2] = pmax ;
     cs[4*4+3] = maxCos ;

     cs[5*4+0] = maxSin2 ;  
     cs[5*4+1] = meanNumberOfPhotons1 ;
     cs[5*4+2] = meanNumberOfPhotons2 ;
     cs[5*4+3] = spare2 ;

     m_genstep->add(cs, m_genstep_itemsize);
}

void CCollector::collectMachineryStep(unsigned code)
{
     m_machinery_count += 1 ;   // 1-based index
     LOG(debug) 
           << " machinery_count " << m_machinery_count ;


     float* ms = m_genstep_values ; 

     uif_t uif ; 
     uif.u = code ; 
     for(unsigned i=0 ; i < m_genstep_itemsize ; i++) ms[i] = uif.f ; 

     m_genstep->add(ms, m_genstep_itemsize);
}




void CCollector::collectPrimary(
               G4double  x0,
               G4double  y0,
               G4double  z0,
               G4double  t0,

               G4double  dir_x,
               G4double  dir_y,
               G4double  dir_z,
               G4double  weight,

               G4double  pol_x,
               G4double  pol_y,
               G4double  pol_z,
               G4double  wavelength,

               unsigned flags_x,
               unsigned flags_y,
               unsigned flags_z,
               unsigned flags_w
          )
{
     float* pr = m_primary_values ; 
     
     pr[0*4+0] = x0 ;
     pr[0*4+1] = y0 ;
     pr[0*4+2] = z0 ;
     pr[0*4+3] = t0 ;

     pr[1*4+0] = dir_x ;
     pr[1*4+1] = dir_y ;
     pr[1*4+2] = dir_z ;
     pr[1*4+3] = weight  ;

     pr[2*4+0] = pol_x ;
     pr[2*4+1] = pol_y ;
     pr[2*4+2] = pol_z ;
     pr[2*4+3] = wavelength  ;

     uif_t flags[4] ;
     flags[0].u = flags_x ;   
     flags[1].u = flags_y ;   
     flags[2].u = flags_z ;   
     flags[3].u = flags_w ;   

     pr[3*4+0] = flags[0].f ;
     pr[3*4+1] = flags[1].f ;
     pr[3*4+2] = flags[2].f ;
     pr[3*4+3] = flags[3].f  ;

     m_primary->add(pr, m_primary_itemsize);
}

 
