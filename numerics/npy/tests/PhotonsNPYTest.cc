#include "NPY.hpp"
#include "Types.hpp"
#include "PhotonsNPY.hpp"
#include "RecordsNPY.hpp"
#include "BoundariesNPY.hpp"

#include "stdlib.h"
#include "assert.h"

int main(int argc, char** argv)
{
    const char* idpath = getenv("IDPATH");
    const char* tag = "1" ;

    NPY<float>* photons = NPY<float>::load("oxcerenkov", tag,"dayabay");
    NPY<short>* records = NPY<short>::load("rxcerenkov", tag,"dayabay");
    NPY<float>* domains = NPY<float>::load("domain","1","dayabay");
    NPY<int>*   idom = NPY<int>::load("idomain","1","dayabay");
    unsigned int maxrec = idom->getValue(0,0,3) ; // TODO: enumerate the k indices 
    assert(maxrec == 10);

    Types types ; 
    types.readFlags("$ENV_HOME/graphics/optixrap/cu/photon.h");
    //types.dumpFlags();
    types.readMaterials(idpath, "GMaterialIndex");
    //types.dumpMaterials();

    RecordsNPY r(records, maxrec);
    r.setTypes(&types);
    r.setDomains(domains);

    PhotonsNPY p(photons);
    p.setTypes(&types);
    p.setRecs(&r);


    if(argc > 1)
    {
       for(unsigned int i=0 ; i < argc-1 ; i++) p.dump(atoi(argv[i+1]));
    }
    else
    {
       p.dumpPhotons("photons", 30);
    }



    return 0 ;
}

