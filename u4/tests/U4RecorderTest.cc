#include "G4RunManager.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4UserRunAction.hh"
#include "G4UserEventAction.hh"
#include "G4UserTrackingAction.hh"
#include "G4UserSteppingAction.hh"

#include "G4SystemOfUnits.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleGun.hh"
#include "G4GeometryManager.hh"

#include "OPTICKS_LOG.hh"
#include "SEvt.hh"
#include "SPath.hh"
#include "SEventConfig.hh"
#include "NP.hh"

#include "U4VolumeMaker.hh"
#include "U4Recorder.hh"
#include "U4Physics.hh"
#include "U4VPrimaryGenerator.h"

struct U4RecorderTest
    : 
    public G4UserRunAction,  
    public G4UserEventAction,
    public G4UserTrackingAction,
    public G4UserSteppingAction,
    public G4VUserPrimaryGeneratorAction,
    public G4VUserDetectorConstruction
{
    static G4ParticleGun* InitGun(); 

    U4Recorder*           fRecorder ; 
    G4ParticleGun*        fGun ;  

    G4VPhysicalVolume* Construct(); 

    void GeneratePrimaries(G4Event* evt); 

    void BeginOfRunAction(const G4Run*);
    void EndOfRunAction(const G4Run*);

    void BeginOfEventAction(const G4Event*);
    void EndOfEventAction(const G4Event*);

    void PreUserTrackingAction(const G4Track*);
    void PostUserTrackingAction(const G4Track*);

    void UserSteppingAction(const G4Step*);

    U4RecorderTest(G4RunManager* runMgr); 
    virtual ~U4RecorderTest(); 
};

G4ParticleGun* U4RecorderTest::InitGun()
{
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition* particle = particleTable->FindParticle("e+");
    LOG(info) << " particle " << particle ; 
    G4ParticleGun* gun = new G4ParticleGun(1) ;   
    gun->SetParticleDefinition(particle);
    gun->SetParticleTime(0.0*CLHEP::ns);
    gun->SetParticlePosition(G4ThreeVector(0.0*CLHEP::cm,0.0*CLHEP::cm,0.0*CLHEP::cm));
    gun->SetParticleMomentumDirection(G4ThreeVector(1.,0.,0.));
    gun->SetParticleEnergy(1.0*MeV); 
    return gun ; 
}

U4RecorderTest::U4RecorderTest(G4RunManager* runMgr)
    :
    fRecorder(new U4Recorder),
    fGun(InitGun())
{
    runMgr->SetUserInitialization((G4VUserDetectorConstruction*)this);
    runMgr->SetUserAction((G4VUserPrimaryGeneratorAction*)this);
    runMgr->SetUserAction((G4UserRunAction*)this);
    runMgr->SetUserAction((G4UserEventAction*)this);
    runMgr->SetUserAction((G4UserTrackingAction*)this);
    runMgr->SetUserAction((G4UserSteppingAction*)this);
    runMgr->Initialize(); 
}

G4VPhysicalVolume* U4RecorderTest::Construct(){ return U4VolumeMaker::WorldBoxOfScintillator( 1000. ); }
void U4RecorderTest::GeneratePrimaries(G4Event* event)
{ 
    fGun->GeneratePrimaryVertex(event); 
    //U4VPrimaryGenerator::GeneratePrimaries(event);   
}

// pass along the message to the recorder
void U4RecorderTest::BeginOfRunAction(const G4Run* run){         fRecorder->BeginOfRunAction(run);   }
void U4RecorderTest::EndOfRunAction(const G4Run* run){           fRecorder->EndOfRunAction(run);     }
void U4RecorderTest::BeginOfEventAction(const G4Event* evt){     fRecorder->BeginOfEventAction(evt); }
void U4RecorderTest::EndOfEventAction(const G4Event* evt){       fRecorder->EndOfEventAction(evt);   }
void U4RecorderTest::PreUserTrackingAction(const G4Track* trk){  fRecorder->PreUserTrackingAction(trk); }
void U4RecorderTest::PostUserTrackingAction(const G4Track* trk){ fRecorder->PostUserTrackingAction(trk); }
void U4RecorderTest::UserSteppingAction(const G4Step* step){     fRecorder->UserSteppingAction(step); }

U4RecorderTest::~U4RecorderTest()
{
    G4GeometryManager::GetInstance()->OpenGeometry(); 
}

/**
G4GeometryManager::OpenGeometry is needed to avoid cleanup warning::

    WARNING - Attempt to delete the physical volume store while geometry closed !
    WARNING - Attempt to delete the logical volume store while geometry closed !
    WARNING - Attempt to delete the solid store while geometry closed !
    WARNING - Attempt to delete the region store while geometry closed !

This below constraint forces instanciating G4RunManager first in order 
to hookup physics before the main instanciation::

    You are instantiating G4UserRunAction BEFORE your G4VUserPhysicsList is
    instantiated and assigned to G4RunManager.
    Such an instantiation is prohibited by Geant4 version 8.0. To fix this problem,
    please make sure that your main() instantiates G4VUserPhysicsList AND
    set it to G4RunManager before instantiating other user action classes
    such as G4UserRunAction.

**/

int main(int argc, char** argv)
{ 
    OPTICKS_LOG(argc, argv); 

    unsigned max_bounce = 9 ; 
    SEventConfig::SetMaxBounce(max_bounce); 
    SEventConfig::SetMaxRecord(max_bounce+1); 
    SEventConfig::SetMaxRec(max_bounce+1); 
    SEventConfig::SetMaxSeq(max_bounce+1); 

    SEvt evt ; 
    //SEvt::AddTorchGenstep();  
    //    needs to be commented when gun running, TODO: make run modes configurable 

    G4RunManager* runMgr = new G4RunManager ; 
    runMgr->SetUserInitialization((G4VUserPhysicsList*)new U4Physics); 
    U4RecorderTest t(runMgr) ;  
    runMgr->BeamOn(1); 
 
    const char* dir = "$TMP/U4RecorderTest" ; 
    evt.saveLabels(dir); 
    evt.save(dir); 
    LOG(info) << evt.desc() ; 

    return 0 ; 
}
