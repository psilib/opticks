#include "PhysicsList.hh"

#include "G4ProcessManager.hh"

// process
#include "G4VUserPhysicsList.hh"
//#include "G4Electron.hh"
//#include "G4OpticalPhoton.hh"
#include "G4BosonConstructor.hh"
#include "G4LeptonConstructor.hh"

#include "G4Scintillation.hh"
#include "G4OpBoundaryProcess.hh"


template <typename T>
PhysicsList<T>::PhysicsList()
       :
       fMaxNumPhotonStep(1000),
       fVerboseLevel(1),
       fCerenkovProcess(NULL),
       fScintillationProcess(NULL),
       fBoundaryProcess(NULL)
{
}


template <typename T>
void PhysicsList<T>::ConstructParticle()
{
    G4LeptonConstructor::ConstructParticle(); 
    G4BosonConstructor::ConstructParticle(); 
}

template <typename T>
void PhysicsList<T>::ConstructProcess()
{
    AddTransportation();
    ConstructEM();
    ConstructOp();
}



// from OpNovicePhysicsList::ConstructEM

#include "G4ComptonScattering.hh"
#include "G4GammaConversion.hh"
#include "G4PhotoElectricEffect.hh"

#include "G4eMultipleScattering.hh"
#include "G4MuMultipleScattering.hh"
#include "G4hMultipleScattering.hh"

#include "G4eIonisation.hh"
#include "G4eBremsstrahlung.hh"
#include "G4eplusAnnihilation.hh"

#include "G4MuIonisation.hh"
#include "G4MuBremsstrahlung.hh"
#include "G4MuPairProduction.hh"

#include "G4hIonisation.hh"


template <typename T>
void PhysicsList<T>::ConstructEM()
{
  theParticleIterator->reset();
  while( (*theParticleIterator)() ){
    G4ParticleDefinition* particle = theParticleIterator->value();
    G4ProcessManager* pmanager = particle->GetProcessManager();
    G4String particleName = particle->GetParticleName();

    if (particleName == "gamma") {
    // gamma
      // Construct processes for gamma
      pmanager->AddDiscreteProcess(new G4GammaConversion());
      pmanager->AddDiscreteProcess(new G4ComptonScattering());
      pmanager->AddDiscreteProcess(new G4PhotoElectricEffect());

    } else if (particleName == "e-") {
    //electron
      // Construct processes for electron
      pmanager->AddProcess(new G4eMultipleScattering(),-1, 1, 1); 
      pmanager->AddProcess(new G4eIonisation(),       -1, 2, 2); 
      pmanager->AddProcess(new G4eBremsstrahlung(),   -1, 3, 3); 

    } else if (particleName == "e+") {
    //positron
      // Construct processes for positron
      pmanager->AddProcess(new G4eMultipleScattering(),-1, 1, 1); 
      pmanager->AddProcess(new G4eIonisation(),       -1, 2, 2); 
      pmanager->AddProcess(new G4eBremsstrahlung(),   -1, 3, 3); 
      pmanager->AddProcess(new G4eplusAnnihilation(),  0,-1, 4); 

    } else if( particleName == "mu+" ||
               particleName == "mu-"    ) { 
    //muon
     // Construct processes for muon
     pmanager->AddProcess(new G4MuMultipleScattering(),-1, 1, 1); 
     pmanager->AddProcess(new G4MuIonisation(),      -1, 2, 2); 
     pmanager->AddProcess(new G4MuBremsstrahlung(),  -1, 3, 3); 
     pmanager->AddProcess(new G4MuPairProduction(),  -1, 4, 4); 

    } else {
      if ((particle->GetPDGCharge() != 0.0) &&
          (particle->GetParticleName() != "chargedgeantino") &&
          !particle->IsShortLived()) {
       // all others charged particles except geantino
       pmanager->AddProcess(new G4hMultipleScattering(),-1,1,1);
       pmanager->AddProcess(new G4hIonisation(),       -1,2,2);
     }   
    }   
  }
}




template <typename T>
void PhysicsList<T>::ConstructOp()
{
    fCerenkovProcess = new T("Cerenkov");
    fCerenkovProcess->SetMaxNumPhotonsPerStep(fMaxNumPhotonStep);
    fCerenkovProcess->SetMaxBetaChangePerStep(10.0);
    fCerenkovProcess->SetTrackSecondariesFirst(true);   
    fCerenkovProcess->SetVerboseLevel(fVerboseLevel);

    fBoundaryProcess = new G4OpBoundaryProcess();

    theParticleIterator->reset();
    while( (*theParticleIterator)() )
    {
        G4ParticleDefinition* particle = theParticleIterator->value();
        G4ProcessManager* pmanager = particle->GetProcessManager();
        G4String particleName = particle->GetParticleName();

        if ( fCerenkovProcess && fCerenkovProcess->IsApplicable(*particle)) 
        {
            pmanager->AddProcess(fCerenkovProcess);
            pmanager->SetProcessOrdering(fCerenkovProcess,idxPostStep);
        }

        if ( fScintillationProcess && fScintillationProcess->IsApplicable(*particle)) 
        {
            pmanager->AddProcess(fScintillationProcess);
            pmanager->SetProcessOrderingToLast(fScintillationProcess, idxAtRest);
            pmanager->SetProcessOrderingToLast(fScintillationProcess, idxPostStep);
        }

        if (particleName == "opticalphoton") 
        {
            G4cout << " AddDiscreteProcess to OpticalPhoton " << G4endl;
            //pmanager->AddDiscreteProcess(fAbsorptionProcess);
            //pmanager->AddDiscreteProcess(fRayleighScatteringProcess);
            //pmanager->AddDiscreteProcess(fMieHGScatteringProcess);
            pmanager->AddDiscreteProcess(fBoundaryProcess);
        }
    }
}



//#include "G4Cerenkov.hh"
#include "L4Cerenkov.hh"
//#include "Cerenkov.hh"

template struct PhysicsList<L4Cerenkov> ; 


