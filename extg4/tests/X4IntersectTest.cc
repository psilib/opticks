/**
X4IntersectTest
=================

Used from script extg4/xxs.sh 

**/
#include "G4Orb.hh"
#include "OPTICKS_LOG.hh"
#include "SSys.hh"
#include "SStr.hh"

#include "X4Intersect.hh"
#include "X4GeometryMaker.hh"

#ifdef WITH_PMTSIM
#include "PMTSim.hh"
#endif

const G4VSolid* MakeSolid(const char* name)
{
    const G4VSolid* solid = nullptr ; 
    if( strcmp(name, "orb") == 0 )
    {
        solid = new G4Orb( name, 100. );
    }
    else if(X4GeometryMaker::CanMake(name))
    {
        solid = X4GeometryMaker::Make(name); 
    }
    else
    {
#ifdef WITH_PMTSIM
        solid = PMTSim::GetSolid(name); 
#endif
    }
    return solid ; 
}

int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 

    const char* geom_default = "pmt_solid" ; 
    const char* geom = SSys::getenvvar("GEOM", geom_default );  

    std::stringstream ss ; 
    ss << "creator:X4IntersectTest" << std::endl ; 
    ss << "geom:" << geom << std::endl ; 
#ifdef WITH_PMTSIM
    ss << "info:WITH_PMTSIM " << std::endl ; 
#else
    ss << "info:noPMTSIM " << std::endl ; 
#endif
    std::string meta = ss.str(); 
    LOG(info) << meta ; 


    std::vector<std::string> names ; 
    SStr::Split(geom,',',names);  

    LOG(info) << " geom " << geom << " names.size " << names.size() ; 

    for(unsigned i=0 ; i < names.size() ; i++)
    {
        const std::string& name = names[i]; 
        const G4VSolid* solid = MakeSolid(name.c_str()); 
        X4Intersect::Scan(solid, name.c_str(), "$TMP/extg4/X4IntersectTest", meta ); 
    }

    return 0 ; 
}

