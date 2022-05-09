#pragma once

#include <string>
#include <vector>
#include "QUDARAP_API_EXPORT.hh"
#include "plog/Severity.h"

/**
QSim
======

The canonical QSim instance is instanciated with CSGOptiX::CSGOptiX

QSim is mostly constant and needs initializing once only 
corresponding to the geometry and the physics process
implementations.  

Contrast with the QEvent with a very different event-by-event lifecycle  

TODO: more modularization, do less directly in QSim

**/

struct NP ; 
template <typename T> struct QTex ; 
template <typename T> struct QBuf ; 
template <typename T> struct QProp ; 

struct qsim ; 

struct QEvent ; 
struct QRng ; 
struct QScint ;
struct QBnd ; 
struct QPrd ; 
struct QMultiFilmLUT;
struct QOptical ; 
struct QEvent ; 
struct QDebug ; 

struct qdebug ; 
struct qstate ; 

struct quad4 ; 
struct quad2 ; 
struct sphoton ; 
union  quad ; 

struct QUDARAP_API QSim
{
    static const plog::Severity LEVEL ; 
    static const char* PREFIX ; 
    static const QSim* INSTANCE ; 
    static const QSim* Get(); 

    static void UploadComponents(const NP* icdf, const NP* bnd, const NP* optical, const char* rindexpath );   
    static void UploadMultiFilmLUT(const NP * multi_film_lut );
 
    QEvent*          event ; 
    const QRng*      rng ;   
    const QScint*    scint ; 
    const QBnd*      bnd ; 
    const QPrd*      prd ; 
    const QOptical*  optical ; 
    const QDebug*    debug_ ; 

    const QProp<float>*  prop ; 
    const QMultiFilmLUT * multi_film;// correspond New PMT optical model;

    const int         pidx ; 
    qsim*             sim ;  
    qsim*           d_sim ;  

    qdebug*           dbg ; 
    qdebug*           d_dbg ; 



    dim3 numBlocks ; 
    dim3 threadsPerBlock ; 

    QSim();

    void init(); 
    void init_sim(); 

    NP* duplicate_dbg_ephoton(unsigned num_photon); 

    qsim* getDevicePtr() const ; 

    char getScintTexFilterMode() const ;
    std::string desc() const ; 

    void configureLaunch16();
    void configureLaunch( unsigned width, unsigned height );
    void configureLaunch2D( unsigned width, unsigned height );
    void configureLaunch1D(unsigned num, unsigned threads_per_block); 
    std::string descLaunch() const ; 


    template<typename T>
    void rng_sequence( dim3 numblocks, dim3 threadsPerBlock, qsim* d_sim, T* d_seq, unsigned ni_tranche, unsigned nv, unsigned ioffset );

    template<typename T>
    void rng_sequence( T* seq, unsigned ni, unsigned nj, unsigned ioffset ); 

    template<typename T>
    void rng_sequence( const char* dir, unsigned ni, unsigned nj, unsigned nk, unsigned ni_tranche_size );


    NP* scint_wavelength( unsigned num_wavelength, unsigned& hd_factor ); 
    NP* cerenkov_wavelength_rejection_sampled( unsigned num_wavelength ); 
    void dump_wavelength(                       float* wavelength, unsigned num_wavelength, unsigned edgeitems=10 ); 

    NP* scint_generate(unsigned num_photon ); 
    NP* cerenkov_generate(unsigned num_photon, unsigned test ); 

    void dump_photon(            quad4* photon, unsigned num_photon, const char* opt="f0,f1,f2,i3", unsigned egdeitems=10 ); 

    void generate_photon(QEvent* evt); 
    void fill_state_0(quad6*  state, unsigned num_state); 
    void fill_state_1(qstate* state, unsigned num_state); 

    NP* quad_launch_generate(unsigned num_quad, unsigned type ); 
    NP* photon_launch_generate(unsigned num_photon, unsigned type ); 


    void photon_launch_mutate(   sphoton* photon, unsigned num_photon, unsigned type ); 

    void mock_propagate( NP* photon, const NP* prd, unsigned type ); 

    unsigned getBoundaryTexWidth() const ;
    unsigned getBoundaryTexHeight() const ;
    const NP* getBoundaryTexSrc() const ; 

    NP* boundary_lookup_all( unsigned width, unsigned height ) ; 
    NP* boundary_lookup_line( float* domain, unsigned num_lookup, unsigned line, unsigned k ) ; 


    template<typename T>
    void prop_lookup(          T* lookup, const T* domain, unsigned domain_width, const std::vector<unsigned>& pids ) ;

    template<typename T>
    void prop_lookup_onebyone( T* lookup, const T* domain, unsigned domain_width, const std::vector<unsigned>& pids ) ;


    
    void multifilm_lookup_all( quad2* sample , quad2* result ,  unsigned width, unsigned height );


};


