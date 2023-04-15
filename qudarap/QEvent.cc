#include <cuda_runtime.h>
#include <sstream>
#include <csignal>

#include "SEvt.hh"
#include "SGeo.hh"
#include "SPath.hh"

#include "scuda.h"
#include "squad.h"
#include "sphoton.h"
#include "srec.h"
#include "sseq.h"
#include "stag.h"
#include "sevent.h"

#include "sqat4.h"
#include "stran.h"
#include "SU.hh"

#include "SComp.h"
#include "SGenstep.hh"
#include "SEvent.hh"
#include "SEvt.hh"
#include "SEventConfig.hh"
#include "NP.hh"
#include "SLOG.hh"

#include "OpticksGenstep.h"

#include "QEvent.hh"
#include "QBuf.hh"
#include "QBuf.hh"
#include "QU.hh"


template struct QBuf<quad6> ; 

const plog::Severity QEvent::LEVEL = SLOG::EnvLevel("QEvent", "DEBUG"); 
QEvent* QEvent::INSTANCE = nullptr ; 
QEvent* QEvent::Get(){ return INSTANCE ; }

sevent* QEvent::getDevicePtr() const
{
    return d_evt ; 
}

/**
QEvent::QEvent
----------------

Instanciation allocates device buffers with sizes configured by SEventConfig

* As selector is only needed CPU side it is not down in sevent.h 
  but it could be in SEvt.hh 

**/

QEvent::QEvent()
    :
    sev(SEvt::Get()),
    selector(sev ? sev->selector : nullptr),
    evt(sev ? sev->evt : nullptr),
    d_evt(QU::device_alloc<sevent>(1,"QEvent::QEvent/sevent")),
    gs(nullptr),
    input_photon(nullptr),
    upload_count(0),
    meta()
{
    INSTANCE = this ; 
    init(); 
}

/**
QEvent::init
--------------

Only configures limits, no allocation yet. Allocation happens in QEvent::setGenstep QEvent::setNumPhoton

HMM: hostside sevent.h instance could reside in SEvt together with selector then hostside setup
can be common between the branches 

**/

void QEvent::init()
{
    LOG_IF(fatal, !sev) << "QEvent instanciated before SEvt instanciated : this is not going to fly " ; 

    assert(sev); 
    assert(evt); 
    assert(selector); 

    LOG(LEVEL) << " QEvent::init calling SEvt::setCompProvider " ; 
    sev->setCompProvider(this);  
}

NP* QEvent::gatherDomain() const { return sev ? sev->gatherDomain() : nullptr ; }


std::string QEvent::desc() const
{
    std::stringstream ss ; 
    ss << evt->desc() << std::endl ;
    std::string s = ss.str();  
    return s ; 
}


void QEvent::setMeta(const char* meta_)
{
    meta = meta_ ;   // std::string
} 

bool QEvent::hasMeta() const 
{
    return meta.empty() == false ; 
}


/**
QEvent::setGenstep
--------------------

Canonically invoked from QSim::simulate and QSim::simtrace just prior to cx->launch 

1. gensteps uploaded to QEvent::init allocated evt->genstep device buffer, 
   overwriting any prior gensteps and evt->num_genstep is set 

2. *count_genstep_photons* calculates the total number of seeds (and photons) by 
   adding the photons from each genstep and setting evt->num_seed

3. *fill_seed_buffer* populates seed buffer using num photons per genstep from genstep buffer

3. invokes setNumPhoton which may allocate records


* HMM: find that without zeroing the seed buffer the seed filling gets messed up causing QEventTest fails 
  doing this in QEvent::init is not sufficient need to do in QEvent::setGenstep.  
  **This is a documented limitation of sysrap/iexpand.h**
 
  So far it seems that no zeroing is needed for the genstep buffer. 

HMM: what about simtrace ? ce-gensteps are very different to ordinary gs 

**/

int QEvent::setGenstep()  // onto device
{
    NP* gs = SEvt::GatherGenstep(); // TODO: review memory handling  
    SEvt::Clear();   // clear the quad6 vector, ready to collect more genstep
    LOG_IF(fatal, gs == nullptr ) << "Must SEvt::AddGenstep before calling QEvent::setGenstep " ;
    //if(gs == nullptr) std::raise(SIGINT); 
    return gs == nullptr ? -1 : setGenstep(gs) ; 
} 


/**
QEvent::setGenstep
-------------------

Recall that even with input photon running, still have gensteps.  
So when the number of gensteps is zero can safely skip this. 

**/


int QEvent::setGenstep(NP* gs_) 
{ 
    gs = gs_ ; 
    SGenstep::Check(gs); 
    evt->num_genstep = gs->shape[0] ; 
    bool not_allocated = evt->genstep == nullptr && evt->seed == nullptr ; 

    LOG(LEVEL) 
        << " evt.num_genstep " << evt->num_genstep 
        << " not_allocated " << ( not_allocated ? "YES" : "NO" ) 
        ; 

    if(not_allocated) 
    {
        device_alloc_genstep() ; 
    }

    LOG(LEVEL) << SGenstep::Desc(gs, 10) ;
 
    bool num_gs_allowed = evt->num_genstep <= evt->max_genstep ;
    LOG_IF(fatal, !num_gs_allowed) << " evt.num_genstep " << evt->num_genstep << " evt.max_genstep " << evt->max_genstep ; 
    assert( num_gs_allowed ); 

    QU::copy_host_to_device<quad6>( evt->genstep, (quad6*)gs->bytes(), evt->num_genstep ); 

    QU::device_memset<int>(   evt->seed,    0, evt->max_photon );

    //count_genstep_photons();   // sets evt->num_seed
    //fill_seed_buffer() ;       // populates seed buffer
    count_genstep_photons_and_fill_seed_buffer();   // combi-function doing what both the above do 


    int gencode0 = SGenstep::GetGencode(gs, 0); // gencode of first genstep   

    if(OpticksGenstep_::IsFrame(gencode0))   // OpticksGenstep_FRAME  (HMM: Obtuse, maybe change to SIMTRACE ?)
    {
        setNumSimtrace( evt->num_seed ); 
    }
    else if(OpticksGenstep_::IsInputPhoton(gencode0))
    {
        setInputPhoton(); 
    }
    else
    {
        setNumPhoton( evt->num_seed );  // photon, rec, record may be allocated here depending on SEventConfig
    }
    upload_count += 1 ; 
    return 0 ; 
}

void QEvent::device_alloc_genstep()
{
    LOG(LEVEL) << " device_alloc genstep and seed " ; 
    evt->genstep = QU::device_alloc<quad6>( evt->max_genstep, "device_alloc_genstep:quad6" ) ; 
    evt->seed    = QU::device_alloc<int>(   evt->max_photon , "device_alloc_genstep:int seed" )  ;
}



/**
QEvent::setInputPhoton
------------------------

This is a private method invoked only from QEvent::setGenstep
which gets the input photon array from SEvt and uploads 
it to the device. 
When the input_photon array is in double precision it is 
narrowed here prior to upload. 

**/

void QEvent::setInputPhoton()
{
    input_photon = SEvt::GetInputPhoton(); 
    LOG_IF(fatal, input_photon == nullptr) 
        << " INCONSISTENT : OpticksGenstep_INPUT_PHOTON by no input photon array " 
        ; 

    assert(input_photon);  
    assert(input_photon->has_shape( -1, 4, 4) ); 
    assert(input_photon->ebyte == 4 || input_photon->ebyte == 8); 

    int num_photon = input_photon->shape[0] ; 
    assert( evt->num_seed == num_photon ); 

    NP* narrow_input_photon = input_photon->ebyte == 8 ? NP::MakeNarrow(input_photon) : input_photon ; 

    setNumPhoton( num_photon ); 
    QU::copy_host_to_device<sphoton>( evt->photon, (sphoton*)narrow_input_photon->bytes(), num_photon ); 
}



int QEvent::setGenstep(quad6* qgs, unsigned num_gs )  // TODO: what uses this ? eliminate ?
{
    NP* gs_ = NP::Make<float>( num_gs, 6, 4 ); 
    gs_->read2( (float*)qgs );   
    return setGenstep( gs_ ); 
}




bool QEvent::hasGenstep() const { return evt->genstep != nullptr ; }
bool QEvent::hasSeed() const {    return evt->seed != nullptr ; }
bool QEvent::hasPhoton() const {  return evt->photon != nullptr ; }
bool QEvent::hasRecord() const { return evt->record != nullptr ; }
bool QEvent::hasRec() const    { return evt->rec != nullptr ; }
bool QEvent::hasSeq() const    { return evt->seq != nullptr ; }
bool QEvent::hasPrd() const    { return evt->prd != nullptr ; }
bool QEvent::hasTag() const    { return evt->tag != nullptr ; }
bool QEvent::hasFlat() const   { return evt->flat != nullptr ; }
bool QEvent::hasHit() const    { return evt->hit != nullptr ; }
bool QEvent::hasSimtrace() const  { return evt->simtrace != nullptr ; }




/**
QEvent::count_genstep_photons
------------------------------

thrust::reduce using strided iterator summing over GPU side gensteps 

**/

extern "C" unsigned QEvent_count_genstep_photons(sevent* evt) ; 
unsigned QEvent::count_genstep_photons()
{
   return QEvent_count_genstep_photons( evt );  
}

/**
QEvent::fill_seed_buffer
---------------------------

Populates seed buffer using the number of photons from each genstep 

The photon seed buffer is a device buffer containing integer indices referencing 
into the genstep buffer. The seeds provide the association between the photon 
and the genstep required to generate it.

**/

extern "C" void QEvent_fill_seed_buffer(sevent* evt ); 
void QEvent::fill_seed_buffer()
{
    QEvent_fill_seed_buffer( evt ); 
}

extern "C" void QEvent_count_genstep_photons_and_fill_seed_buffer(sevent* evt ); 
void QEvent::count_genstep_photons_and_fill_seed_buffer()
{
    QEvent_count_genstep_photons_and_fill_seed_buffer( evt ); 
}





NP* QEvent::getGenstep() const 
{
    return gs ; 
}

NP* QEvent::getInputPhoton() const 
{
    return input_photon ; 
}




/**
QEvent::gatherGenstepFromDevice
---------------------------------

Gensteps originate on host and are uploaded to device, so downloading
them from device is not usually done. 

**/

NP* QEvent::gatherGenstepFromDevice() const 
{
    NP* a = NP::Make<float>( evt->num_genstep, 6, 4 ); 
    QU::copy_device_to_host<quad6>( (quad6*)a->bytes(), evt->genstep, evt->num_genstep ); 
    return a ; 
}


NP* QEvent::gatherSeed() const 
{
    bool has_seed = hasSeed() ; 
    LOG_IF(fatal, !has_seed) << " gatherSeed called when there is no such array, use SEventConfig::SetCompMask to avoid " ; 
    if(!has_seed) return nullptr ;  
    NP* s = NP::Make<int>( evt->num_seed ); 
    QU::copy_device_to_host<int>( (int*)s->bytes(), evt->seed, evt->num_seed ); 
    return s ; 
}


/**
QEvent::gatherPhoton(NP* p) :  mutating API
-------------------------------------------
**/

void QEvent::gatherPhoton(NP* p) const 
{
    bool expected_shape =  p->has_shape(evt->num_photon, 4, 4) ; 
    LOG(expected_shape ? LEVEL : fatal) << "[ evt.num_photon " << evt->num_photon << " p.sstr " << p->sstr() << " evt.photon " << evt->photon ; 
    assert(expected_shape ); 
    QU::copy_device_to_host<sphoton>( (sphoton*)p->bytes(), evt->photon, evt->num_photon ); 
    LOG(LEVEL) << "] evt.num_photon " << evt->num_photon  ; 
}

NP* QEvent::gatherPhoton() const 
{
    NP* p = NP::Make<float>( evt->num_photon, 4, 4);
    gatherPhoton(p); 
    return p ; 
}




void QEvent::gatherSimtrace(NP* t) const 
{
    LOG(LEVEL) << "[ evt.num_simtrace " << evt->num_simtrace << " t.sstr " << t->sstr() << " evt.simtrace " << evt->simtrace ; 
    assert( t->has_shape(evt->num_simtrace, 4, 4) ); 
    QU::copy_device_to_host<quad4>( (quad4*)t->bytes(), evt->simtrace, evt->num_simtrace ); 
    LOG(LEVEL) << "] evt.num_simtrace " << evt->num_simtrace  ; 
}
NP* QEvent::gatherSimtrace() const 
{
    bool has_simtrace = hasSimtrace(); 
    LOG_IF(LEVEL, !has_simtrace) << " getSimtrace called when there is no such array, use SEventConfig::SetCompMask to avoid " ;
    if(!has_simtrace) return nullptr ;  
    NP* t = NP::Make<float>( evt->num_simtrace, 4, 4);
    gatherSimtrace(t); 
    return t ; 
}

void QEvent::gatherSeq(NP* seq) const 
{
    bool has_seq = hasSeq(); 
    if(!has_seq) return ; 
    LOG(LEVEL) << "[ evt.num_seq " << evt->num_seq << " seq.sstr " << seq->sstr() << " evt.seq " << evt->seq ; 
    assert( seq->has_shape(evt->num_seq, 2) ); 
    QU::copy_device_to_host<sseq>( (sseq*)seq->bytes(), evt->seq, evt->num_seq ); 
    LOG(LEVEL) << "] evt.num_seq " << evt->num_seq  ; 
}

NP* QEvent::gatherSeq() const 
{
    bool has_seq = hasSeq(); 
    LOG_IF(LEVEL, !has_seq) << " gatherSeq called when there is no such array, use SEventConfig::SetCompMask to avoid " ; 
    if(!has_seq) return nullptr ;
  
    NP* seq = sev->makeSeq(); 

    gatherSeq(seq); 
    return seq ; 
}

NP* QEvent::gatherPrd() const 
{
    bool has_prd = hasPrd(); 
    LOG_IF(LEVEL, !has_prd) << " gatherPrd called when there is no such array, use SEventConfig::SetCompMask to avoid " ; 
    if(!has_prd) return nullptr ;
  
    NP* prd = sev->makePrd(); 
    LOG(LEVEL) << " evt.num_prd " << evt->num_prd ; 
    QU::copy_device_to_host<quad2>( (quad2*)prd->bytes(), evt->prd, evt->num_prd ); 
    return prd ; 
}

NP* QEvent::gatherTag() const 
{
    bool has_tag = hasTag() ; 
    LOG_IF(LEVEL, !has_tag) << " gatherTag called when there is no such array, use SEventConfig::SetCompMask to avoid " ; 
    if(!has_tag) return nullptr ;
  
    NP* tag = sev->makeTag(); 
    LOG(LEVEL) << " evt.num_tag " << evt->num_tag << " tag.desc " << tag->desc() ; 
    QU::copy_device_to_host<stag>( (stag*)tag->bytes(), evt->tag, evt->num_tag ); 
    return tag ; 
}

NP* QEvent::gatherFlat() const 
{
    bool has_flat = hasFlat(); 
    LOG_IF(LEVEL, !has_flat) << " gatherFlat called when there is no such array, use SEventConfig::SetCompMask to avoid " ; 
    if(!has_flat) return nullptr ;
  
    NP* flat = sev->makeFlat(); 
    LOG(LEVEL) << " evt.num_flat " << evt->num_flat << " flat.desc " << flat->desc() ; 
    QU::copy_device_to_host<sflat>( (sflat*)flat->bytes(), evt->flat, evt->num_flat ); 
    return flat ; 
}


NP* QEvent::gatherRecord() const 
{
    bool has_record = hasRecord() ; 
    LOG_IF(LEVEL, !has_record) << " gatherRecord called when there is no such array, use SEventConfig::SetCompMask to avoid " ; 
    if(!has_record) return nullptr ;  

    NP* r = sev->makeRecord(); 

    LOG(LEVEL) << " evt.num_record " << evt->num_record ; 
    QU::copy_device_to_host<sphoton>( (sphoton*)r->bytes(), evt->record, evt->num_record ); 
    return r ; 
}

NP* QEvent::gatherRec() const 
{
    bool has_rec = hasRec(); 
    LOG_IF(LEVEL, !has_rec ) << " gatherRec called when there is no such array, use SEventConfig::SetCompMask to avoid " ; 
    if(!has_rec) return nullptr ;  

    NP* r = sev->makeRec(); 

    LOG(LEVEL) 
        << " evt.num_photon " << evt->num_photon 
        << " evt.max_rec " << evt->max_rec 
        << " evt.num_rec " << evt->num_rec  
        << " evt.num_photon*evt.max_rec " << evt->num_photon*evt->max_rec  
        ;

    assert( evt->num_photon*evt->max_rec == evt->num_rec );  

    QU::copy_device_to_host<srec>( (srec*)r->bytes(), evt->rec, evt->num_rec ); 
    return r ; 
}


unsigned QEvent::getNumHit() const 
{
    assert( evt->photon ); 
    assert( evt->num_photon ); 

    evt->num_hit = SU::count_if_sphoton( evt->photon, evt->num_photon, *selector );    

    LOG(LEVEL) << " evt.photon " << evt->photon << " evt.num_photon " << evt->num_photon << " evt.num_hit " << evt->num_hit ;  
    return evt->num_hit ; 
}

/**
QEvent::gatherHit
------------------

1. count *evt.num_hit* passing the photon *selector* 
2. allocate *evt.hit* GPU buffer
3. copy_if from *evt.photon* to *evt.hit* using the photon *selector*
4. host allocate the NP hits array
5. copy hits from device to the host NP hits array 
6. free *evt.hit* on device
7. return NP hits array to caller, who becomes owner of the array 

Note that the device hits array is allocated and freed for each launch.  
This is due to the expectation that the number of hits will vary greatly from launch to launch 
unlike the number of photons which is expected to be rather similar for most launches other than 
remainder last launches. 

The alternative to this dynamic "busy" handling of hits would be to reuse a fixed hits buffer
sized to max_photons : that however seems unpalatable due it always doubling up GPU memory for 
photons and hits.  

hitmask metadata was formerly placed on the hit array, 
subsequently moved to domain_meta as domain should 
always be present, unlike hits. 

**/

NP* QEvent::gatherHit() const 
{
    // hasHit at this juncture is misleadingly always false, 
    // because the hits array is derived by *gatherHit_* which  selects from the photons 

    bool has_photon = hasPhoton(); 

    LOG_IF(LEVEL, !has_photon) << " gatherHit called when there is no photon array " ; 
    if(!has_photon) return nullptr ; 

    assert( evt->photon ); 
    assert( evt->num_photon ); 
    evt->num_hit = SU::count_if_sphoton( evt->photon, evt->num_photon, *selector );    

    LOG(LEVEL) 
         << " evt.photon " << evt->photon 
         << " evt.num_photon " << evt->num_photon 
         << " evt.num_hit " << evt->num_hit
         << " selector.hitmask " << selector->hitmask
         << " SEventConfig::HitMask " << SEventConfig::HitMask()
         << " SEventConfig::HitMaskLabel " << SEventConfig::HitMaskLabel()
         ;  

    NP* hit = evt->num_hit > 0 ? gatherHit_() : nullptr ; 

    return hit ; 
}

NP* QEvent::gatherHit_() const 
{
    evt->hit = QU::device_alloc<sphoton>( evt->num_hit, "QEvent::gatherHit_:sphoton" ); 

    SU::copy_if_device_to_device_presized_sphoton( evt->hit, evt->photon, evt->num_photon,  *selector );

    NP* hit = NP::Make<float>( evt->num_hit, 4, 4 ); 

    QU::copy_device_to_host<sphoton>( (sphoton*)hit->bytes(), evt->hit, evt->num_hit );

    QU::device_free<sphoton>( evt->hit ); 

    evt->hit = nullptr ; 
    LOG(LEVEL) << " hit.sstr " << hit->sstr() ; 

    return hit ; 
}



std::string QEvent::getMeta() const 
{     
    return meta ; 
}
NP* QEvent::gatherComponent(unsigned comp) const 
{
    LOG(LEVEL) << "[ comp " << comp ; 
    unsigned mask = SEventConfig::CompMask(); 
    bool proceed = (mask & comp) != 0 ; 
    NP* a = proceed ? gatherComponent_(comp) : nullptr ;
    LOG(LEVEL) << "[ comp " << comp << " proceed " << proceed << " a " <<  a ; 
    return a ; 
}
NP* QEvent::gatherComponent_(unsigned comp) const 
{
    NP* a = nullptr ; 
    switch(comp)
    {   
        case SCOMP_GENSTEP:   a = getGenstep()     ; break ;   
        case SCOMP_DOMAIN:    a = gatherDomain()      ; break ;   
        case SCOMP_INPHOTON:  a = getInputPhoton() ; break ;   

        case SCOMP_PHOTON:    a = gatherPhoton()   ; break ;   
        case SCOMP_RECORD:    a = gatherRecord()   ; break ;   
        case SCOMP_REC:       a = gatherRec()      ; break ;   
        case SCOMP_SEQ:       a = gatherSeq()      ; break ;   
        case SCOMP_PRD:       a = gatherPrd()      ; break ;   
        case SCOMP_SEED:      a = gatherSeed()     ; break ;   
        case SCOMP_HIT:       a = gatherHit()      ; break ;   
        case SCOMP_SIMTRACE:  a = gatherSimtrace() ; break ;   
        case SCOMP_TAG:       a = gatherTag()      ; break ;   
        case SCOMP_FLAT:      a = gatherFlat()     ; break ;   
        // gather downloads from device, get access from host 
    }   
    return a ; 
}



/**
QEvent::setNumPhoton
---------------------

At the first call when evt.photon is nullptr allocation on device is done. 

Canonically invoked internally from QEvent::setGenstep but may be invoked 
directly from "friendly" photon only tests without use of gensteps.  

Sets evt->num_photon asserts that is within allowed *evt->max_photon* and calls *uploadEvt*

This assumes that the number of photons for subsequent launches does not increase 
when collecting records : that is ok as running with records is regarded as debugging. 
**/

void QEvent::setNumPhoton(unsigned num_photon )
{
    sev->setNumPhoton(num_photon); 
    if( evt->photon == nullptr ) device_alloc_photon();  
    uploadEvt(); 
}
void QEvent::setNumSimtrace(unsigned num_simtrace)
{
    sev->setNumSimtrace(num_simtrace); 
    if( evt->simtrace == nullptr ) device_alloc_simtrace(); 
    uploadEvt(); 
}

/**
QEvent::device_alloc_photon
----------------------------

HMM: record buffer should be : evt->max_record * evt->max_photon ? 

**/

void QEvent::device_alloc_photon()
{
    evt->photon  = evt->max_photon > 0 ? QU::device_alloc_zero<sphoton>( evt->max_photon, "max_photon*sizeof(sphoton)" ) : nullptr ; 

    evt->record  = evt->max_record > 0 ? QU::device_alloc_zero<sphoton>( evt->max_photon * evt->max_record, "max_photon*max_record*sizeof(sphoton)" ) : nullptr ; 
    evt->rec     = evt->max_rec    > 0 ? QU::device_alloc_zero<srec>(    evt->max_photon * evt->max_rec   , "max_photon*max_rec*sizeof(srec)"    ) : nullptr ; 
    evt->seq     = evt->max_seq    > 0 ? QU::device_alloc_zero<sseq>(    evt->max_photon * evt->max_seq   , "max_photon*max_seq*sizeof(sseq)"    ) : nullptr ; 
    evt->prd     = evt->max_prd    > 0 ? QU::device_alloc_zero<quad2>(   evt->max_photon * evt->max_prd   , "max_photon*max_prd*sizeof(quad2)"    ) : nullptr ; 
    evt->tag     = evt->max_tag   == 1 ? QU::device_alloc_zero<stag>(    evt->max_photon                  , "max_photon*sizeof(stag)"    ) : nullptr ; 
    evt->flat    = evt->max_flat  == 1 ? QU::device_alloc_zero<sflat>(   evt->max_photon                  , "max_photon*sizeof(sflat)"   ) : nullptr ; 

    LOG(LEVEL) 
        << " evt.max_photon " << evt->max_photon 
        << " evt.num_photon " << evt->num_photon 
        << " evt.num_record " << evt->num_record 
        << " evt.num_rec    " << evt->num_rec 
        << " evt.num_seq    " << evt->num_seq
        << " evt.num_prd    " << evt->num_prd
        << " evt.num_tag    " << evt->num_tag
        << " evt.num_flat   " << evt->num_flat
        ;
}
 
void QEvent::device_alloc_simtrace()
{
    evt->simtrace = QU::device_alloc<quad4>( evt->max_simtrace, "max_simtrace" ) ; 
    LOG(LEVEL) 
        << " evt.num_simtrace " << evt->num_simtrace 
        << " evt.max_simtrace " << evt->max_simtrace
        ;
}


/**
QEvent::uploadEvt 
--------------------

Copies host side *evt* instance (with updated num_genstep and num_photon) to device side  *d_evt*.  
Note that the evt->genstep and evt->photon pointers are not updated, so the same buffers are reused for each launch. 

**/

void QEvent::uploadEvt()
{
    LOG(LEVEL) << std::endl << evt->desc() ; 
    QU::copy_host_to_device<sevent>(d_evt, evt, 1 );  
}

unsigned QEvent::getNumPhoton() const
{
    return evt->num_photon ; 
}
unsigned QEvent::getNumSimtrace() const
{
    return evt->num_simtrace ; 
}



extern "C" void QEvent_checkEvt(dim3 numBlocks, dim3 threadsPerBlock, sevent* evt, unsigned width, unsigned height ) ; 

void QEvent::checkEvt() 
{ 
    unsigned width = getNumPhoton() ; 
    unsigned height = 1 ; 
    LOG(info) << " width " << width << " height " << height ; 

    dim3 numBlocks ; 
    dim3 threadsPerBlock ; 
    QU::ConfigureLaunch( numBlocks, threadsPerBlock, width, height ); 
 
    assert( d_evt ); 
    QEvent_checkEvt(numBlocks, threadsPerBlock, d_evt, width, height );   
}


