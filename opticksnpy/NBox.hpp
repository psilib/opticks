#pragma once

#include "NGLM.hpp"
#include "NNode.hpp"
#include "NPart.hpp"

#include "NPY_API_EXPORT.hh"

/*

nbox 
======

Currently two flavors of box use nbox class

#. CSG_BOX (positioned box with single dimension control) 
#. CSG_BOX3 (unplaced box with 3 dimension control) 

Perhaps CSG_BOX3 will usurp CSG_BOX at which point can 
change the enum.

*/


struct NPY_API nbox : nnode 
{
    float operator()(float x, float y, float z) const ;

    float sdf1(float x, float y, float z) ;
    float sdf2(float x, float y, float z) ;

    void adjustToFit(const nbbox& container_bb, float scale);

    nbbox bbox() const ;

    unsigned par_nsurf() const ;
    glm::vec3 par_pos(const glm::vec2& uv, unsigned surf ) const ;


    glm::vec3 gseedcenter();

    void pdump(const char* msg="nbox::pdump", int verbosity=1);

    glm::vec3 center ; 

    bool is_box3 ; 

};

// only methods that are specific to boxes 
// and need to override the nnode need to be here 




inline NPY_API void init_box(nbox& b, const nquad& p )
{
    b.param = p ; 
    b.center.x = p.f.x ; 
    b.center.y = p.f.y ; 
    b.center.z = p.f.z ; 
    b.is_box3 = false ; 
}
inline NPY_API void init_box3(nbox& b, const nquad& p )
{
    b.param = p ; 
    b.center.x = 0 ; 
    b.center.y = 0 ; 
    b.center.z = 0 ; 
    b.is_box3 = true ; 
}


inline NPY_API nbox make_box(const nquad& p)
{
    nbox n ; 
    nnode::Init(n,CSG_BOX) ; 
    init_box(n, p );
    return n ;
}
inline NPY_API nbox make_box3(const nquad& p)
{
    nbox n ; 
    nnode::Init(n,CSG_BOX3) ; 
    init_box3(n, p );
    return n ;
}


inline NPY_API nbox make_box(float x, float y, float z, float w)
{
    nquad param ;
    param.f =  {x,y,z,w} ;
    return make_box( param ); 
}
inline NPY_API nbox make_box3(float x, float y, float z)
{
    nquad param ;
    param.f =  {x,y,z,0} ;
    return make_box3( param ); 
}

