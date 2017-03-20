#pragma once

#include "NNode.hpp"

struct nplane ; 
struct ndisc ; 
struct npart ;
struct nbbox ; 


#include "NPY_API_EXPORT.hh"

struct NPY_API nsphere : nnode {

    // NO CTOR

    float x();
    float y();
    float z();
    float radius();
    float costheta(float z);

    double operator()(double px, double py, double pz) ;
    nbbox bbox();

    npart part();
    static ndisc intersect(nsphere& a, nsphere& b);

    // result of intersect allows partitioning 
    npart zrhs(const ndisc& dsc); // +z to the right  
    npart zlhs(const ndisc& dsc);  


};


inline NPY_API nsphere make_nsphere(float x, float y, float z, float w)
{
    nsphere n ; nnode::Init(n,CSG_SPHERE) ; n.param.x = x ; n.param.y = y ; n.param.z = z ; n.param.w = w ; return n ;
}

inline NPY_API nsphere* make_nsphere_ptr(float x, float y, float z, float w)
{
    nsphere* n = new nsphere ; nnode::Init(*n,CSG_SPHERE) ; n->param.x = x ; n->param.y = y ; n->param.z = z ; n->param.w = w ; return n ;
}
inline NPY_API nsphere* make_nsphere_ptr(const nvec4& param)
{
    nsphere* n = new nsphere ; nnode::Init(*n,CSG_SPHERE) ; n->param.x = param.x ; n->param.y = param.y ; n->param.z = param.z ; n->param.w = param.w ; return n ;
}



inline NPY_API nsphere make_nsphere(const nvec4& p)
{
    nsphere n ; nnode::Init(n,CSG_SPHERE) ; n.param.x = p.x ; n.param.y = p.y ; n.param.z = p.z ; n.param.w = p.w ; return n ;
}


