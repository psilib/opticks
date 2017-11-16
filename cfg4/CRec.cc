#include <climits>

#include "CG4.hh"
#include "CG4Ctx.hh"
#include "CRec.hh"
#include "CStp.hh"
#include "Format.hh"

#include "PLOG.hh"

CRec::CRec(CG4* g4)
   :
    m_g4(g4),
    m_ctx(g4->getCtx()),
    m_ok(g4->getOpticks())
{
}
 
void CRec::setOrigin(const G4ThreeVector& origin)
{
    m_origin = origin ; 
}

void CRec::clearStp()
{
    if(m_ctx._dbgrec) 
    LOG(info) << "[--dbgrec] CRec::clearStp"
              << " clearing " << m_stp.size() << " stps "
              ;
 
    m_stp.clear();
}


#ifdef USE_CUSTOM_BOUNDARY
bool CRec::add(const G4Step* step, int step_id, DsG4OpBoundaryProcessStatus boundary_status,  CStage::CStage_t stage )
#else
bool CRec::add(const G4Step* step, int step_id,   G4OpBoundaryProcessStatus boundary_status,  CStage::CStage_t stage )
#endif
{
    m_stp.push_back(new CStp(step, step_id, boundary_status, stage, m_origin));
    return false ; 
}


#ifdef USE_CUSTOM_BOUNDARY
void CRec::add(const G4Step* step, int step_id, DsG4OpBoundaryProcessStatus boundary_status, unsigned premat, unsigned postmat, unsigned preflag, unsigned postflag,  CStage::CStage_t stage, int action)
#else
void CRec::add(const G4Step* step, int step_id,  G4OpBoundaryProcessStatus boundary_status, unsigned premat, unsigned postmat, unsigned preflag, unsigned postflag, CStage::CStage_t stage, int action)
#endif
{
    m_stp.push_back(new CStp(step, step_id, boundary_status, premat, postmat, preflag, postflag, stage, action, m_origin ));
}

unsigned CRec::getNumStps()
{
    return m_stp.size();
}

CStp* CRec::getStp(unsigned index)
{
    return index < m_stp.size() ? m_stp[index] : NULL ; 
}

void CRec::dump(const char* msg)
{
    unsigned nstp = m_stp.size();
    LOG(info) << msg  
              << " record_id " << m_ctx._record_id
              << " " << Format(m_origin, "origin")
              << " nstp " << nstp 
              << " " << ( nstp > 0 ? m_stp[0]->origin() : "-" ) 
              ; 


    for( unsigned i=0 ; i < nstp ; i++)
        std::cout << "(" << std::setw(2) << i << ") " << m_stp[i]->description() << std::endl ;  

}

