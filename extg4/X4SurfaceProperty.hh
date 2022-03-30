#pragma once

#include "X4_API_EXPORT.hh"
#include "G4Version.hh"
#include "G4SurfaceProperty.hh"


/**
X4SurfaceProperty
==================

**/

struct X4_API X4SurfaceProperty
{
    static bool IsOpticksSupported(G4SurfaceType type); 
    static const char* Name(G4SurfaceType type);
    static G4SurfaceType Type(const char* name);

    static const char* dielectric_metal_ ; 
    static const char* dielectric_dielectric_ ; 
    static const char* dielectric_LUT_ ; 
#if (G4VERSION_NUMBER >= 1042)
    static const char* dielectric_LUTDAVIS_ ; 
#endif
    static const char* dielectric_dichroic_ ; 
    static const char* firsov_ ; 
    static const char* x_ray_ ; 
};


inline const char* X4SurfaceProperty::dielectric_metal_      = "dielectric_metal" ; 
inline const char* X4SurfaceProperty::dielectric_dielectric_ = "dielectric_dielectric" ; 
inline const char* X4SurfaceProperty::dielectric_LUT_        = "dielectric_LUT" ; 
#if (G4VERSION_NUMBER >= 1042)
inline const char* X4SurfaceProperty::dielectric_LUTDAVIS_   = "dielectric_LUTDAVIS" ; 
#endif
inline const char* X4SurfaceProperty::dielectric_dichroic_   = "dielectric_dichroic" ; 
inline const char* X4SurfaceProperty::firsov_                = "firsov" ; 
inline const char* X4SurfaceProperty::x_ray_                 = "x_ray" ; 


inline const char* X4SurfaceProperty::Name(G4SurfaceType type) // static
{
   const char* t = NULL ; 
   switch(type)
   {
       case dielectric_metal      : t=dielectric_metal_      ; break ; 
       case dielectric_dielectric : t=dielectric_dielectric_ ; break ; 
       case dielectric_LUT        : t=dielectric_LUT_        ; break ; 
#if (G4VERSION_NUMBER >= 1042)
       case dielectric_LUTDAVIS   : t=dielectric_LUTDAVIS_   ; break ; 
#endif
       case dielectric_dichroic   : t=dielectric_dichroic_   ; break ; 
       case firsov                : t=firsov_                ; break ;
       case x_ray                 : t=x_ray_                 ; break ;
   }
   return t ; 
}    

inline G4SurfaceType X4SurfaceProperty::Type(const char* name) // static
{
    G4SurfaceType type=dielectric_dielectric ; 
    if(strcmp(name,dielectric_metal_)==0 )       type=dielectric_metal ; 
    if(strcmp(name,dielectric_dielectric_)==0 )  type=dielectric_dielectric ; 
    if(strcmp(name,dielectric_LUT_)==0 )         type=dielectric_LUT ; 
#if (G4VERSION_NUMBER >= 1042)
    if(strcmp(name,dielectric_LUTDAVIS_)==0 )    type=dielectric_LUTDAVIS ; 
#endif
    if(strcmp(name,dielectric_dichroic_)==0 )    type=dielectric_dichroic ; 
    if(strcmp(name,firsov_)==0 )                 type=firsov ; 
    if(strcmp(name,x_ray_)==0 )                  type=x_ray ; 
    return type ; 
}


inline bool X4SurfaceProperty::IsOpticksSupported(G4SurfaceType type) // static
{
    return type == dielectric_metal || type == dielectric_dielectric ; 
}

