#include <cstddef>

#include "OpZeroer.hh"

#include "OpticksSwitches.h"
#include "Opticks.hh"   
#include "OpticksEvent.hh"   // okc-

// npy-
#include "Timer.hpp"
#include "PLOG.hh"
#include "NPY.hpp"

// cudawrap-
#include "CResource.hh"
#include "CBufSpec.hh"

// thrustrap-
#include "TBuf.hh"

// optixrap-
#include "OContext.hh"
#include "OEvent.hh"
#include "OBuf.hh"


#define TIMER(s) \
    { \
       if(m_ok)\
       {\
          Timer& t = *(m_ok->getTimer()) ;\
          t((s)) ;\
       }\
    }




OpZeroer::OpZeroer(Opticks* ok, OEvent* oevt)  
   :
     m_ok(ok),
     m_oevt(oevt),
     m_ocontext(oevt->getOContext())
{
}


void OpZeroer::zeroRecords()
{
#ifdef WITH_RECORD
    LOG(info)<<"OpZeroer::zeroRecords" ;

    if( m_ocontext->isInterop() )
    {    
        zeroRecordsViaOpenGL();
    }    
    else if ( m_ocontext->isCompute() )
    {    
        zeroRecordsViaOptiX();
    }    
#endif
}


void OpZeroer::zeroRecordsViaOpenGL()
{
#ifdef WITH_RECORD
    OpticksEvent* evt = m_ok->getEvent();

    NPY<short>* record = evt->getRecordData(); 

    CResource r_rec( record->getBufferId(), CResource::W );

    CBufSpec s_rec = r_rec.mapGLToCUDA<short>() ;

    s_rec.Summary("OpZeroer::zeroRecordsViaOpenGL(CBufSpec)s_rec");

    TBuf trec("trec", s_rec );

    trec.zero();

    r_rec.unmapGLToCUDA();

    TIMER("zeroRecordsViaOpenGL"); 
#endif
}


void OpZeroer::zeroRecordsViaOptiX()
{
#ifdef WITH_RECORD
    OBuf* record = m_oevt->getRecordBuf() ;

    CBufSpec s_rec = record->bufspec();

    s_rec.Summary("OpZeroer::zeroRecordsViaOptiX(CBufSpec)s_rec");

    TBuf trec("trec", s_rec );

    trec.zero();

    TIMER("zeroRecordsViaOptiX"); 
#endif
}


