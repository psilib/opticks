#pragma once

#include <string>
#include <vector>
class G4LogicalVolume ; 
class G4VPhysicalVolume ; 
class G4Material ; 

#include "G4ThreeVector.hh"
#include "U4_API_EXPORT.hh"
#include "plog/Severity.h"

struct U4_API U4VolumeMaker
{
    static const plog::Severity LEVEL ; 
    static const char* GEOM ; 
    static std::string Desc(); 
    
    static G4VPhysicalVolume* Make(); // sensitive to GEOM envvar 
    static G4VPhysicalVolume* Make(const char* name); 
    static G4VPhysicalVolume* Make_(const char* name); 

    static G4VPhysicalVolume* MakePhysical(const char* name); 
    static G4VPhysicalVolume* MakePhysicalList_(const char* name); 
    static G4VPhysicalVolume* MakePhysicalOne_(const char* name); 

    static G4LogicalVolume*   MakeLogical(const char* name); 
    static void MakeLogical(std::vector<G4LogicalVolume*>& lvs , const char* names_ ) ; 

    static void AddPlacement( G4LogicalVolume* mother, G4LogicalVolume* lv,  double tx, double ty, double tz ); 
    static void AddPlacement( G4LogicalVolume* mother, const char* name   ,  double tx, double ty, double tz ); 

    static G4VPhysicalVolume* WrapLVOffset(    G4LogicalVolume* lv, double tx, double ty, double tz ); 
    static G4VPhysicalVolume* WrapLVTranslate( G4LogicalVolume* lv, double tx, double ty, double tz ); 




    static G4VPhysicalVolume* WorldBox( double halfside, const char* mat="Vacuum" ); 
    static G4VPhysicalVolume* WorldBox( double halfside, G4Material* material  ); 
    static G4VPhysicalVolume* BoxOfScintillator( double halfside ); 
    static G4VPhysicalVolume* BoxOfScintillator( double halfside, const char* prefix, G4LogicalVolume* mother_lv ); 
    static G4VPhysicalVolume* Box(double halfside, G4Material* material, const char* prefix, G4LogicalVolume* mother_lv ); 


    static constexpr const char* U4VolumeMaker_RaindropRockAirWater_HALFSIDE = "U4VolumeMaker_RaindropRockAirWater_HALFSIDE" ; 
    static constexpr const char* U4VolumeMaker_RaindropRockAirWater_FACTOR   = "U4VolumeMaker_RaindropRockAirWater_FACTOR" ; 
    static void RaindropRockAirWater_Configure( double& rock_halfside, double& air_halfside, double& water_radius ); 

    static G4VPhysicalVolume* RaindropRockAirWater();  
    static G4VPhysicalVolume* RaindropRockAirWater2();
 
    static G4LogicalVolume* Orb_( double radius,   const char* mat, const char* prefix=nullptr ); 
    static G4LogicalVolume* Box_( double halfside, const char* mat, const char* prefix=nullptr ); 

    static G4VPhysicalVolume* Wrap( G4LogicalVolume* lv, double halfside  ); 


    static G4VPhysicalVolume* WrapLVGrid( G4LogicalVolume* lv, int nx, int ny, int nz  ); 
    static G4VPhysicalVolume* WrapLVGrid( std::vector<G4LogicalVolume*>& lvs, int nx, int ny, int nz  ); 




    static const char* GridName(const char* prefix, int ix, int iy, int iz, const char* suffix); 
    static std::string Desc( const G4ThreeVector& tla ); 
};


