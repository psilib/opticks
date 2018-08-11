
#include <sstream>
#include <iomanip>

#include "Ctx.hh"

#include "G4Event.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4OpticalPhoton.hh"

#include "G4ThreeVector.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"

#include "PLOG.hh"


void Ctx::setEvent(const G4Event* event)
{
    _event = event ; 
    _event_id = event->GetEventID() ; 
    //_event_total += 1 ;
    //_event_track_count = 0 ; 

    _track = NULL ; 
    _track_id = -1 ; 

    LOG(info) << " _event_id " << _event_id ; 
}

void Ctx::setTrack(const G4Track* track)
{
    _track = track ; 
    _track_id = track->GetTrackID() - 1 ;

    _step = NULL ; 
    _step_id = -1 ;  

    G4ParticleDefinition* particle = track->GetDefinition();

    _track_particle_name = track->GetDynamicParticle()->GetParticleDefinition()->GetParticleName(); 

    _track_optical = particle == G4OpticalPhoton::OpticalPhotonDefinition() ;

    _track_pdg_encoding = particle->GetPDGEncoding();

    if(_track_optical)
    {
        const_cast<G4Track*>(track)->UseGivenVelocity(true);
    }

/*
    LOG(info) << " _track_id " << _track_id 
              << " _track_pdg_encoding " << _track_pdg_encoding
              << " _track_particle_name " << _track_particle_name
              ;
*/
}

void Ctx::setStep(const G4Step* step)
{  
    _step = step ; 
    _step_id = _track->GetCurrentStepNumber() - 1 ;

    _track_step_count += 1 ;
    
    const G4StepPoint* pre = _step->GetPreStepPoint() ;
    //const G4StepPoint* post = _step->GetPostStepPoint() ;

    if(_step_id == 0) _step_origin = pre->GetPosition();

/*
    LOG(info) 
        << " _step_id " << _step_id 
        ;  
*/

}



std::string Ctx::Format(const G4ThreeVector& vec, const char* msg, unsigned int fwid)
{   
    std::stringstream ss ; 
    ss 
       << " " 
       << msg 
       << "[ "
       << std::fixed
       << std::setprecision(3)  
       << std::setw(fwid) << vec.x()
       << std::setw(fwid) << vec.y()
       << std::setw(fwid) << vec.z()
       << "] "
       ;
    return ss.str();
}

std::string Ctx::Format(const G4StepPoint* point, const char* msg )
{
    const G4ThreeVector& pos = point->GetPosition();
    const G4ThreeVector& dir = point->GetMomentumDirection();
    const G4ThreeVector& pol = point->GetPolarization();
    std::stringstream ss ; 
    ss 
       << " " 
       << msg 
       << Format(pos, "pos", 10)
       << Format(dir, "dir", 10)
       << Format(pol, "pol", 10)
       ;
    return ss.str();
}

std::string Ctx::Format(const G4Step* step, const char* msg )
{
    const G4StepPoint* pre = step->GetPreStepPoint() ;
    const G4StepPoint* post = step->GetPostStepPoint() ;
    std::stringstream ss ; 
    ss 
       << " " 
       << msg 
       << std::endl 
       << Format(pre,  "pre ")
       << std::endl 
       << Format(post, "post")
       ; 
    return ss.str();
}




