#pragma once

#include "CLHEP/Random/RandomEngine.h"

struct NP ; 

/**
OpticksRandom
==============

Controlling the random sequence returned by G4UniformRand using 
precooked array of randoms generated by curand together with  
extracts from cfg4/CAlignEngine.cc

Primary control *OpticksRandom::setSequenceIndex* 
to set the photon_id sequence of randoms to consume from  
and then use *G4UniformRand* as normal which will internally use the 
below *OpticksRandom::flat* method while this is enabled.

**/

struct OpticksRandom : public CLHEP::HepRandomEngine
{
    friend struct OpticksRandomTest ; 
    static const char* NAME ; 
    static OpticksRandom* INSTANCE ; 
    static OpticksRandom* Get(); 

    const NP*                m_seq;  
    const double*            m_seq_values ; 
    int                      m_seq_ni ; 
    int                      m_seq_nv ; 
    int                      m_seq_index ; 

    NP*                      m_cur ; 
    int*                     m_cur_values ; 
    bool                     m_recycle ; 

    CLHEP::HepRandomEngine*  m_default ;

    OpticksRandom(const NP* seq); 

    virtual ~OpticksRandom(); 

    void setSequenceIndex(int seq_index);  


    // mandatory CLHEP::HepRandomEngine methods
    double flat();
    void flatArray(const int size, double* vect);
    void setSeed(long seed, int);
    void setSeeds(const long * seeds, int); 
    void saveStatus( const char filename[] = "Config.conf") const ;
    void restoreStatus( const char filename[] = "Config.conf" ) ;
    void showStatus() const ;
    std::string name() const ;

    // internals
    private:
        std::string desc() const ; 
        void enable(); 
        void disable(); 
        void dump(unsigned n=10); 


}; 

