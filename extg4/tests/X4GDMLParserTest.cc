/**





**/

#include "X4GDMLParser.hh"

#include "OPTICKS_LOG.hh"
#include "BFile.hh"
#include "X4.hh"
#include "X4CSG.hh"

#include "G4Box.hh"
#include "G4Orb.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4Trd.hh"
#include "G4Polycone.hh"
#include "G4Cons.hh"
#include "G4Ellipsoid.hh"
#include "G4Torus.hh"

#include "G4UnionSolid.hh"
#include "G4IntersectionSolid.hh"
#include "G4SubtractionSolid.hh"

#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"


#define USE_CONE 1

// make_solid from ana/x019.cc : which was 
// start portion generated by nnode::to_g4code 
G4VSolid* make_solid()
{ 
    G4VSolid* d = new G4Ellipsoid("PMT_20inch_inner_solid_1_Ellipsoid0x4c91130", 249.000000, 249.000000, 179.000000, -179.000000, 179.000000) ; // 3

#ifdef USE_CONE
    // see x018_torus_hyperboloid_plt.py
    // In [26]: run x018_torus_hyperboloid_plt.py
    // x.f  f :                 Cons : array([   0.    , -193.7543]) : [44.99, 83.9935, 25.245702570206376] 

    G4double pRmin1 = 0. ;          // inside radius at  -fDz
    G4double pRmax1 = 44.99 ;       // outside radius at -fDz  
    G4double pRmin2 = 0. ;          // inside radius at  +fDz
    G4double pRmax2 =  83.9935  ;   // outside radius at +fDz  
    G4double pDz = 25.245702570206376  ;   // half length in z
    G4double pSPhi = 0   ; 
    G4double pDPhi = CLHEP::twopi ;

    G4VSolid* f = new G4Cons("cons", pRmin1, pRmax1, pRmin2, pRmax2, pDz, pSPhi, pDPhi ); 
    G4ThreeVector B(0.000000,0.000000, -193.7543);
#else
    G4VSolid* g = new G4Tubs("PMT_20inch_inner_solid_2_Tube0x4c91210", 0.000000, 75.951247, 23.782510, 0.000000, CLHEP::twopi) ; // 4
    G4VSolid* i = new G4Torus("PMT_20inch_inner_solid_2_Torus0x4c91340", 0.000000, 52.010000, 97.000000, -0.000175, CLHEP::twopi) ; // 4    
    G4ThreeVector A(0.000000,0.000000,-23.772510);
    G4VSolid* f = new G4SubtractionSolid("PMT_20inch_inner_solid_part20x4cb2d80", g, i, NULL, A) ; // 3
    G4ThreeVector B(0.000000,0.000000,-195.227490);
#endif
    
    G4VSolid* c = new G4UnionSolid("PMT_20inch_inner_solid_1_20x4cb30f0", d, f, NULL, B) ; // 2
    G4VSolid* k = new G4Tubs("PMT_20inch_inner_solid_3_EndTube0x4cb2fc0", 0.000000, 45.010000, 57.510000, 0.000000, CLHEP::twopi) ; // 2
    
    G4ThreeVector C(0.000000,0.000000,-276.500000);
    G4VSolid* b = new G4UnionSolid("PMT_20inch_inner_solid0x4cb32e0", c, k, NULL, C) ; // 1

#ifdef USE_CONE
    return b ;
#else
    G4VSolid* m = new G4Tubs("Inner_Separator0x4cb3530", 0.000000, 254.000000, 92.000000, 0.000000, CLHEP::twopi) ; // 1
    G4ThreeVector D(0.000000,0.000000,92.000000);
    G4VSolid* a = new G4SubtractionSolid("PMT_20inch_inner2_solid0x4cb3870", b, m, NULL, D) ; // 0
    return a ; 
#endif
} 
// end portion generated by nnode::to_g4code 


int main( int argc , char** argv )
{
    OPTICKS_LOG(argc, argv);

    //const char* exename = PLOG::instance->args.exename() ; 

    G4VSolid* solid = make_solid() ; 

    //std::string csgpath = BFile::FormPath(X4::X4GEN_DIR, exename) ; 
    //X4CSG::Serialize( solid, csgpath.c_str() ) ;

    bool refs = false ; // add pointer refs : false because already present
 
    X4GDMLParser::Write( solid, NULL, refs ) ; // to stdout 
    X4GDMLParser::Write( solid, "/tmp/out.gdml", refs ) ; // to file
    std::string gdml = X4GDMLParser::ToString(solid, refs) ; 
    LOG(fatal) << gdml ; 


    return 0 ; 
}
