#pragma once

struct C4 { 
  char x ; 
  char y ;
  char z ;
  unsigned char w ; 
}; 

union C4U { 
   C4 c4 ; 
   unsigned u ;  
};  


#include <sstream>
#include <string>
#include <iostream>
#include <iomanip>

inline std::string C4U_desc( const C4U& c4u )
{
    std::stringstream ss ; 
    ss << " C4U ( " 
       << std::setw(5) << int(c4u.c4.x)  
       << std::setw(5) << int(c4u.c4.y)   
       << std::setw(5) << int(c4u.c4.z)   
       << std::setw(5) << unsigned(c4u.c4.w)
       << " ) "
       ;   
    std::string s = ss.str(); 
    return s ; 
}

inline std::string C4U_desc( unsigned u )
{
    C4U c4u ; 
    c4u.u = u ; 
    return C4U_desc( c4u ); 
}

