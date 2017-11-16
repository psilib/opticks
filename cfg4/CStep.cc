#include "CFG4_BODY.hh"

#include "G4SystemOfUnits.hh"
#include "G4StepPoint.hh"
#include "G4Step.hh"

#include "OpticksQuadrant.h"
#include "CStep.hh"

const G4Step* CStep::getStep() const 
{
    return m_step ; 
}
unsigned int CStep::getStepId() const 
{
    return m_step_id ; 
}



unsigned CStep::PreQuadrant(const G4Step* step) // static 
{
    const G4StepPoint* pre  = step->GetPreStepPoint() ; 
    const G4ThreeVector& pos = pre->GetPosition();
    return 
                  (  pos.x() > 0.f ? unsigned(QX) : 0u ) 
                   |   
                  (  pos.y() > 0.f ? unsigned(QY) : 0u ) 
                   |   
                  (  pos.z() > 0.f ? unsigned(QZ) : 0u )
                  ;   
}



double CStep::PreGlobalTime(const G4Step* step) // static
{
    const G4StepPoint* point  = step->GetPreStepPoint() ; 
    return point ? point->GetGlobalTime()/ns : -1 ;
}
double CStep::PostGlobalTime(const G4Step* step) // static
{
    const G4StepPoint* point  = step->GetPostStepPoint() ; 
    return point ? point->GetGlobalTime()/ns : -1 ;
}


const G4Material* CStep::PreMaterial( const G4Step* step) // static
{
    const G4StepPoint* pre  = step->GetPreStepPoint() ; 
    const G4Material* preMat  = pre->GetMaterial() ;
    return preMat ; 
}

const G4Material* CStep::PostMaterial( const G4Step* step) // static
{
    const G4StepPoint* post = step->GetPostStepPoint() ; 
    const G4Material* postMat  = post->GetMaterial() ;
    return postMat ; 
}



CStep::CStep(const G4Step* step, unsigned int step_id) 
   :
   m_step(NULL),
   m_step_id(step_id)
{
   m_step = new G4Step(*step) ;
}

CStep::~CStep()
{
   delete m_step ; 
}


