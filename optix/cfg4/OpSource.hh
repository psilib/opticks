#pragma once

#include "G4VPrimaryGenerator.hh"
#include "G4ParticleMomentum.hh"

#include "G4Threading.hh"
#include "G4Cache.hh"

class G4ParticleDefinition ; 
class G4SPSPosDistribution ;
class G4SPSAngDistribution ;
class G4SPSEneDistribution ;
class G4SPSRandomGenerator ;

class TorchStepNPY ; 



class OpSource: public G4VPrimaryGenerator 
{
  public:
    OpSource(TorchStepNPY* torch);
  private:
    void init();
    void configure();
  public:
    ~OpSource();
    void GeneratePrimaryVertex(G4Event *evt);
  public:
    void SetVerbosity(G4int);
  public:
    void SetParticleDefinition(G4ParticleDefinition* definition);
    void SetNumberOfParticles(G4int i);
    void SetParticleTime(G4double time);
    void SetParticleCharge(G4double charge);
    void SetParticlePolarization(G4ThreeVector polarization);
    void setIncidentSphereSPolarized(bool spol); 
  public:
    G4int                 GetNumberOfParticles() const ;
    G4ParticleDefinition* GetParticleDefinition() const ;
    G4ThreeVector         GetParticlePolarization() const ;
    G4ThreeVector         GetParticlePosition() const ;
    G4ThreeVector         GetParticleMomentumDirection() const ;
    G4double              GetParticleEnergy() const ;
    G4double              GetParticleTime() const;
private:
    // residents 
    TorchStepNPY*         m_torch ;

    G4SPSPosDistribution* m_posGen;
    G4SPSAngDistribution* m_angGen;
    G4SPSEneDistribution* m_eneGen;
    G4SPSRandomGenerator* m_ranGen;

    struct part_prop_t 
    {
        G4ParticleMomentum momentum_direction; 
        G4double           energy; 
        G4ThreeVector      position; 
        part_prop_t();
    };

    G4Cache<part_prop_t>  m_pp;

    G4int                 m_num;
    bool                  m_isspol ; 
    G4ParticleDefinition* m_definition;
    G4double              m_charge;
    G4double              m_time;
    G4ThreeVector         m_polarization;
    G4int                 m_verbosityLevel;
    G4Mutex               m_mutex;
};


inline OpSource::OpSource(TorchStepNPY* torch)  
    :
    m_torch(torch),
    m_posGen(NULL),
    m_angGen(NULL),
    m_eneGen(NULL),
    m_ranGen(NULL),
	m_num(1),
    m_isspol(false),
	m_charge(0.0),
	m_time(0.0),
	m_polarization(1.0,0.0,0.0),
	m_verbosityLevel(0)
{
    init();
}



inline void OpSource::SetNumberOfParticles(G4int num) 
{
    m_num = num;
}
inline void OpSource::SetParticleTime(G4double time) 
{
    m_time = time;
}
inline void OpSource::SetParticleCharge(G4double charge) 
{
    m_charge = charge;
}
inline void OpSource::SetParticlePolarization(G4ThreeVector polarization) 
{
    m_polarization = polarization ;
}

inline void OpSource::setIncidentSphereSPolarized(bool isspol)
{
    m_isspol = isspol ; 
}




inline G4ParticleDefinition* OpSource::GetParticleDefinition() const 
{
    return m_definition;
}
inline G4int OpSource::GetNumberOfParticles() const 
{
    return m_num ;
}
inline G4double OpSource::GetParticleTime() const 
{
    return m_time;
}
inline G4ThreeVector OpSource::GetParticlePolarization() const 
{
    return m_polarization;
}



inline G4ThreeVector OpSource::GetParticlePosition() const 
{
    return m_pp.Get().position;
}
inline G4ThreeVector OpSource::GetParticleMomentumDirection() const 
{
    return m_pp.Get().momentum_direction;
}
inline G4double OpSource::GetParticleEnergy() const 
{
    return m_pp.Get().energy;
}




