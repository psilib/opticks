#include "NPY.hpp"
#include "VecNPY.hpp"
#include "G4StepNPY.hpp"
#include <iostream>
#include "assert.h"


void test_ctor()
{
    std::vector<int> shape = {2,2} ;
    std::vector<float> data = {1.f,2.f,3.f,4.f}  ;
    std::string metadata = "{}";

    NPY npy(shape,data,metadata) ;
    std::cout << npy.description("npy") << std::endl ; 
}

void test_path()
{
    std::string path = NPY::path("cerenkov", "1");
    std::cout << path << std::endl ; 
}

void test_load()
{
    NPY* npy = NPY::load("cerenkov","1");
    std::cout << npy->description("npy") << std::endl ; 
}

void test_load_missing()
{
    NPY* npy = NPY::load("cerenkov","missing");
    if(npy) std::cout << npy->description("npy") << std::endl ; 
}

void test_g4stepnpy()
{
    NPY* npy = NPY::load("cerenkov","1");
    G4StepNPY* step = new G4StepNPY(npy);   
    step->dump("G4StepNPY");
}

void test_getData()
{
    NPY* npy = NPY::load("cerenkov","1");
    float* data = npy->getFloats();

    for(unsigned int i=0 ; i < 16 ; i++ )
    {
        uif_t uif ;
        uif.f = *(data+i) ;
        printf(" %3u : %15f f   %15d i  %15u u  \n", i, uif.f, uif.i, uif.u );
    }

    char* raw = (char*)data ;
    for(unsigned int i=0 ; i < 16 ; i++ )
    {
        char c = *(raw+i) ;
        printf(" %3u : %d i   %x x \n", i, c, c);
    }
    std::cout << npy->description("npy") << std::endl ; 
}


void test_VecNPY()
{   
    NPY* npy = NPY::load("cerenkov","1");

    VecNPY v(npy,1,0);  // [:,1,0:3]
    v.dump("vecNPY"); 
}




int main()
{
    //test_ctor();
    //test_path();
    //test_load();
    //test_load_missing();
    //test_g4stepnpy();
    //test_getData();

    test_VecNPY();

    return 0 ;
}
