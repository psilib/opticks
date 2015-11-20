#include "NPart.hpp"
#include "NLog.hpp"


void test_p0()
{
    npart p ; 
    p.zero();
    p.dump("p0"); 
}


void test_p1()
{
    npart p ; 
    p.q0.f = { 0.f, 1.f, 2.f, 3.f };
    p.q3.u.w = 101 ; 
    p.q2.i.w = -101 ; 
    p.dump("p1"); 
}


int main(int argc, char** argv)
{
    NLog nl("part.log","info");
    nl.configure(argc, argv, "/tmp");

    test_p0();
    test_p1();


    return 0 ; 
}




