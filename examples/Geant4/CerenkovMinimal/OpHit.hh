#pragma once

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4ThreeVector.hh"

struct OpHit : public G4VHit
{
    static OpHit* MakeDummyHit(); 

    G4double      ene;     
    G4double      tim;     
    G4ThreeVector pos; 
    G4ThreeVector dir; 
    G4ThreeVector pol; 
};

typedef G4THitsCollection<OpHit> OpHitCollection;

