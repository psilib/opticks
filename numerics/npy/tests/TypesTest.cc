#include "Types.hpp"

#include "stdlib.h"
#include "assert.h"
#include "stdio.h"

int main(int argc, char** argv)
{
    const char* idpath = getenv("IDPATH");

    Types types ; 
    types.readFlags("$ENV_HOME/graphics/ggeoview/cu/photon.h");
    types.dumpFlags();
    types.readMaterials(idpath, "GMaterialIndexLocal.json");
    types.dumpMaterials();

    types.getHistoryStringTest();
    types.getMaterialStringTest();
  

    return 0 ;
}

