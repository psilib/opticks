
static __device__
void csg_bounds_sphere(const quad& q0, optix::Aabb* aabb, optix::Matrix4x4* tr  )
{
    float radius = q0.f.w;
    float3 mn = make_float3( q0.f.x - radius, q0.f.y - radius, q0.f.z - radius );
    float3 mx = make_float3( q0.f.x + radius, q0.f.y + radius, q0.f.z + radius );

    //float3 mn = make_float3( -800.f, -800.f , -800.f );
    //float3 mx = make_float3(  800.f,  800.f ,  800.f );

    Aabb tbb(mn, mx);
    if(tr) transform_bbox( &tbb, tr );  

    aabb->include(tbb);

}
 

static __device__
void csg_intersect_sphere(const quad& q0, const float& tt_min, float4& tt, const float3& ray_origin, const float3& ray_direction )
{
    // when an intersection is found between the ray and the sphere 
    // with parametric t greater than the tmin parameter
    // tt is set to the parametric t of the intersection
    // and corresponding tt_normal is set

    float3 center = make_float3(q0.f);
    float radius = q0.f.w;

    float3 O = ray_origin - center;
    float3 D = ray_direction;

    float b = dot(O, D);
    float c = dot(O, O)-radius*radius;
    float disc = b*b-c;

    float sdisc = disc > 0.f ? sqrtf(disc) : 0.f ;   // ray has segment within sphere for sdisc > 0.f 
    float root1 = -b - sdisc ;
    float root2 = -b + sdisc ;  // root2 > root2 always

    float tt_cand = sdisc > 0.f ? ( root1 > tt_min ? root1 : root2 ) : tt_min ; 

    if(tt_cand > tt_min)
    {        
        tt.x = (O.x + tt_cand*D.x)/radius ; 
        tt.y = (O.y + tt_cand*D.y)/radius ; 
        tt.z = (O.z + tt_cand*D.z)/radius ; 
        tt.w = tt_cand ; 
    }
}


static __device__
void csg_bounds_box(const quad& q0, optix::Aabb* aabb, optix::Matrix4x4* tr  )
{
    const float hside = q0.f.w ; 
    const float3 bmin = make_float3(q0.f.x - hside, q0.f.y - hside, q0.f.z - hside ); 
    const float3 bmax = make_float3(q0.f.x + hside, q0.f.y + hside, q0.f.z + hside ); 

    Aabb tbb(bmin, bmax);
    if(tr) transform_bbox( &tbb, tr );  

    aabb->include(tbb);
}

static __device__
void csg_intersect_box(const quad& q0, const float& tt_min, float4& tt, const float3& ray_origin, const float3& ray_direction )
{
   const float hside = q0.f.w ; 
   const float3 bmin = make_float3(q0.f.x - hside, q0.f.y - hside, q0.f.z - hside ); 
   const float3 bmax = make_float3(q0.f.x + hside, q0.f.y + hside, q0.f.z + hside ); 

   const float3 bcen = make_float3(q0.f.x, q0.f.y, q0.f.z) ;    

   float3 idir = make_float3(1.f)/ray_direction ; 

   // the below t-parameter float3 are intersects with the x, y and z planes of
   // the three axis slab planes through the box bmin and bmax  

   float3 t0 = (bmin - ray_origin)*idir;      //  intersects with bmin x,y,z slab planes
   float3 t1 = (bmax - ray_origin)*idir;      //  intersects with bmax x,y,z slab planes 

   float3 near = fminf(t0, t1);               //  bmin or bmax intersects closest to origin  
   float3 far  = fmaxf(t0, t1);               //  bmin or bmax intersects farthest from origin 

   float t_near = fmaxf( near );              //  furthest near intersect              
   float t_far  = fminf( far );               //  closest far intersect 


  // rtPrintf(" bmin %f %f %f ", bmin.x, bmin.y, bmin.z );
  /*
     rtPrintf(" ray_origin %f %f %f ray_direction %f %f %f idir %f %f %f \n ", 
           ray_origin.x,    ray_origin.y, ray_origin.z, 
           ray_direction.x, ray_direction.y, ray_direction.z,
           idir.x, idir.y,  idir.z 
       );

   rtPrintf(" idir %f %f %f t0 %f %f %f t1 %f %f %f \n",
         idir.x, idir.y, idir.z,  
         t0.x, t0.y, t0.z, 
         t1.x, t1.y, t1.z
      );

   rtPrintf(" near %f %f %f -> t_near %f   far %f %f %f -> t_far %f   \n",
         near.x, near.y, near.z, t_near,   
         far.x,  far.y,  far.z, t_far 
    );

*/

   bool along_x = ray_direction.x != 0.f && ray_direction.y == 0.f && ray_direction.z == 0.f ;
   bool along_y = ray_direction.x == 0.f && ray_direction.y != 0.f && ray_direction.z == 0.f ;
   bool along_z = ray_direction.x == 0.f && ray_direction.y == 0.f && ray_direction.z != 0.f ;

   bool in_x = ray_origin.x > bmin.x && ray_origin.x < bmax.x  ;
   bool in_y = ray_origin.y > bmin.y && ray_origin.y < bmax.y  ;
   bool in_z = ray_origin.z > bmin.z && ray_origin.z < bmax.z  ;

   bool has_intersect ;
   if(     along_x) has_intersect = in_y && in_z ;
   else if(along_y) has_intersect = in_x && in_z ; 
   else if(along_z) has_intersect = in_x && in_y ; 
   else             has_intersect = ( t_far > t_near && t_far > 0.f ) ;  // segment of ray intersects box, at least one is ahead

   if( has_intersect ) 
   {
       //  just because the ray intersects the box doesnt 
       //  mean its a usable intersect, there are 3 possibilities
       //
       //                t_near       t_far   
       //
       //                  |           |
       //        -----1----|----2------|------3---------->
       //                  |           |
       //
       //

       float tt_cand = tt_min < t_near ?  t_near : ( tt_min < t_far ? t_far : tt_min ) ; 

       //rtPrintf(" intersect_box : t_near %f t_far %f tt %f tt_min %f \n", t_near, t_far, tt, tt_min  );

       float3 p = ray_origin + tt_cand*ray_direction - bcen ; 
       float3 pa = make_float3(fabs(p.x), fabs(p.y), fabs(p.z)) ;

       float3 n = make_float3(0.f) ;
       if(      pa.x >= pa.y && pa.x >= pa.z ) n.x = copysignf( 1.f , p.x ) ;              
       else if( pa.y >= pa.x && pa.y >= pa.z ) n.y = copysignf( 1.f , p.y ) ;              
       else if( pa.z >= pa.x && pa.z >= pa.y ) n.z = copysignf( 1.f , p.z ) ;              

       if(tt_cand > tt_min)
       {
           tt.x = n.x ;
           tt.y = n.y ;
           tt.z = n.z ;
           tt.w = tt_cand ; 
       }
   }
}

static __device__
void csg_intersect_part(unsigned partIdx, const float& tt_min, float4& tt  )
{
    Part pt = partBuffer[partIdx] ; 
    unsigned partType = pt.partType() ; 
    unsigned gtransformIdx = pt.gtransformIdx() ;  //  gtransformIdx is 1-based, 0 meaning None

    if(gtransformIdx == 0)
    {
        switch(partType)
        {
            case CSG_SPHERE: csg_intersect_sphere(pt.q0,tt_min, tt, ray.origin, ray.direction )  ; break ; 
            case CSG_BOX:    csg_intersect_box(   pt.q0,tt_min, tt, ray.origin, ray.direction )  ; break ; 
        }
    }
    else
    {
        unsigned trIdx   = 2*(gtransformIdx-1) ; 
        unsigned iritIdx = trIdx + 1  ;       

        if(iritIdx >= tranBuffer.size())
        { 
            rtPrintf("##csg_intersect_part ABORT iritIdx %3u overflows tranBuffer.size \n", iritIdx );
            return ;  
        }

        optix::Matrix4x4 tr = tranBuffer[trIdx] ; 
        optix::Matrix4x4 irit = tranBuffer[iritIdx] ; 


        float4 origin    = make_float4( ray.origin.x, ray.origin.y, ray.origin.z, 1.f );           // w=1 for position  
        float4 direction = make_float4( ray.direction.x, ray.direction.y, ray.direction.z, 0.f );  // w=0 for vector

        // bring ray into local frame of the geometry, using inverse transform
        origin    = origin * irit ; 
        direction = direction * irit ; 

        float3 ray_origin = make_float3( origin.x, origin.y, origin.z );
        float3 ray_direction = make_float3( direction.x, direction.y, direction.z );

        // intersect using local frame assuming code
        switch(partType)
        {
            case CSG_SPHERE: csg_intersect_sphere(pt.q0,tt_min, tt, ray_origin, ray_direction )  ; break ; 
            case CSG_BOX:    csg_intersect_box(   pt.q0,tt_min, tt, ray_origin, ray_direction )  ; break ; 
        }

        //   transforming local frame results back to original frame  
        //
        //       what about the parametric tt.w the parametric t distance ?
        //       what about intersection point, not used ? 
        //
        //  :google:`ray tracer transform rays`

        float4 tt_normal = make_float4( tt.x, tt.y, tt.z , 0.f );
        tt_normal = tt_normal * tr ;   // should be inverse tranpose of T ? but i dont scale, so inverse transpose T = T  

        tt.x = tt_normal.x ; 
        tt.y = tt_normal.y ; 
        tt.z = tt_normal.z ; 
    }
}


