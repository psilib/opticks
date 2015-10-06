#include "GMesh.hh"
#include "GMeshFixer.hh"
#include "GBuffer.hh"

#include "float.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"
#include "stringutil.hpp"

#include <iomanip>
#include <algorithm>

#include "numpy.hpp"

#include <boost/log/trivial.hpp>
#define LOG BOOST_LOG_TRIVIAL
// trace/debug/info/warning/error/fatal

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


const char* GMesh::vertices     = "vertices" ;
const char* GMesh::normals      = "normals" ;
const char* GMesh::colors       = "colors" ;
const char* GMesh::texcoords    = "texcoords" ;

const char* GMesh::indices      = "indices" ;
const char* GMesh::nodes        = "nodes" ;
const char* GMesh::boundaries   = "boundaries" ;
const char* GMesh::sensors      = "sensors" ;

const char* GMesh::center_extent = "center_extent" ;
const char* GMesh::bbox           = "bbox" ;
const char* GMesh::transforms     = "transforms" ;
const char* GMesh::meshes         = "meshes" ;
const char* GMesh::nodeinfo       = "nodeinfo" ;


void GMesh::nameConstituents(std::vector<std::string>& names)
{
    names.push_back(vertices); 
    names.push_back(normals); 
    names.push_back(colors); 
    names.push_back(texcoords); 

    names.push_back(indices); 
    names.push_back(nodes); 
    names.push_back(boundaries); 
    names.push_back(sensors); 

    names.push_back(center_extent); 
    names.push_back(bbox); 
    names.push_back(transforms); 
    names.push_back(meshes); 
    names.push_back(nodeinfo); 
}

GMesh::GMesh(GMesh* other) 
     :
     m_index(other->getIndex()),
     m_vertices(other->getVertices()),
     m_vertices_buffer(other->getVerticesBuffer()),
     m_num_vertices(other->getNumVertices()),
     m_faces(other->getFaces()),
     m_indices_buffer(other->getIndicesBuffer()),
     m_num_faces(other->getNumFaces()),
     m_colors(other->getColors()),
     m_colors_buffer(other->getColorsBuffer()),
     m_texcoords(other->getTexcoords()),
     m_texcoords_buffer(other->getTexcoordsBuffer()),
     //m_num_colors(other->getNumColors()),
     m_normals(other->getNormals()),
     m_normals_buffer(other->getNormalsBuffer()),
     m_nodes(other->getNodes()),
     m_nodes_buffer(other->getNodesBuffer()),
     m_boundaries(other->getBoundaries()),
     m_boundaries_buffer(other->getBoundariesBuffer()),
     m_center_extent_buffer(other->getCenterExtentBuffer()),
     m_bbox_buffer(other->getBBoxBuffer()),
     m_num_solids(other->getNumSolids()),
     m_num_solids_selected(other->getNumSolidsSelected()),
     m_transforms_buffer(other->getTransformsBuffer()),
     m_meshes_buffer(other->getMeshesBuffer()),
     m_nodeinfo_buffer(other->getNodeInfoBuffer()),
     m_geocode(other->getGeoCode()),
     GDrawable()
{
   updateBounds();
   nameConstituents(m_names);
}


int GMesh::g_instance_count = 0 ;



GMesh::GMesh(unsigned int index, 
             gfloat3* vertices, 
             unsigned int num_vertices, 
             guint3* faces, 
             unsigned int num_faces, 
             gfloat3* normals, 
             gfloat2* texcoords
            ) 
        :
      m_index(index),
      m_vertices(NULL),
      m_vertices_buffer(NULL),
      m_num_vertices(num_vertices), 
      m_faces(NULL),
      m_indices_buffer(NULL),
      m_nodes(NULL),           // nodes and boundaries are somwhat confusing : use only with subclasses like GMergedMesh
      m_nodes_buffer(NULL),
      m_boundaries(NULL),
      m_boundaries_buffer(NULL),
      m_sensors(NULL),
      m_sensors_buffer(NULL),
      m_num_faces(num_faces),
      m_colors(NULL),
      m_colors_buffer(NULL),
      m_texcoords(NULL),
      m_texcoords_buffer(NULL),
      m_low(NULL),
      m_high(NULL),
      m_dimensions(NULL),
      m_center(NULL),
      m_model_to_world(NULL),
      m_version(NULL),
      m_extent(0.f),
      m_center_extent(NULL),
      m_center_extent_buffer(NULL),
      m_bbox(NULL),
      m_bbox_buffer(NULL),
      m_normals(NULL),
      m_normals_buffer(NULL),
      m_num_solids(0),
      m_num_solids_selected(0),
      m_transforms(NULL),
      m_transforms_buffer(NULL),
      m_meshes(NULL),
      m_meshes_buffer(NULL),
      m_nodeinfo(NULL),
      m_nodeinfo_buffer(NULL),
      m_geocode('T'),
      GDrawable()
{
   // not yet taking ownership, depends on continued existance of data source 

   g_instance_count += 1 ; 

   //printf("GMesh::GMesh  index %d g_instance_count %d \n", index, g_instance_count );

   setVertices(vertices);
   setFaces(faces);
   setNormals(normals);
   setTexcoords(texcoords);
   updateBounds();
   nameConstituents(m_names);
}





void GMesh::allocate()
{
    unsigned int numVertices = getNumVertices();
    unsigned int numFaces = getNumFaces();
    unsigned int numSolids = getNumSolids();

    assert(numVertices > 0 && numFaces > 0 && numSolids > 0);

    setVertices(new gfloat3[numVertices]); 
    setNormals( new gfloat3[numVertices]);
    setColors(  new gfloat3[numVertices]);
    setTexcoords( NULL );  

    setColor(0.5,0.5,0.5);  // starting point mid-grey, change in traverse 2nd pass

    // consolidate into guint4 

    setFaces(        new guint3[numFaces]);

    // TODO: consolidate into uint4 with one spare
    setNodes(        new unsigned int[numFaces]);
    setBoundaries(   new unsigned int[numFaces]);
    setSensors(      new unsigned int[numFaces]);


    setCenterExtent(new gfloat4[numSolids]);
    setBBox(new gbbox[numSolids]);
    setMeshes(new unsigned int[numSolids]);
    setNodeInfo(new guint4[numSolids]);
    setTransforms(new float[numSolids*16]);

}






GBuffer* GMesh::getBuffer(const char* name)
{
    if(strcmp(name, vertices) == 0)     return m_vertices_buffer ; 
    if(strcmp(name, normals) == 0)      return m_normals_buffer ; 
    if(strcmp(name, colors) == 0)       return m_colors_buffer ; 
    if(strcmp(name, texcoords) == 0)    return m_texcoords_buffer ; 

    if(strcmp(name, indices) == 0)      return m_indices_buffer ; 
    if(strcmp(name, nodes) == 0)        return m_nodes_buffer ; 
    if(strcmp(name, boundaries) == 0)   return m_boundaries_buffer ; 
    if(strcmp(name, sensors) == 0)      return m_sensors_buffer ; 

    if(strcmp(name, center_extent) == 0)   return m_center_extent_buffer ; 
    if(strcmp(name, bbox) == 0)            return m_bbox_buffer ; 
    if(strcmp(name, transforms) == 0)      return m_transforms_buffer ; 
    if(strcmp(name, meshes) == 0)          return m_meshes_buffer ; 
    if(strcmp(name, nodeinfo) == 0)        return m_nodeinfo_buffer ; 

    return NULL ;
}


void GMesh::setBuffer(const char* name, GBuffer* buffer)
{
    if(strcmp(name, vertices) == 0)     setVerticesBuffer(buffer) ; 
    if(strcmp(name, normals) == 0)      setNormalsBuffer(buffer) ; 
    if(strcmp(name, colors) == 0)       setColorsBuffer(buffer) ; 
    if(strcmp(name, texcoords) == 0)    setTexcoordsBuffer(buffer) ; 

    if(strcmp(name, indices) == 0)      setIndicesBuffer(buffer) ; 
    if(strcmp(name, nodes) == 0)        setNodesBuffer(buffer) ; 
    if(strcmp(name, boundaries) == 0)   setBoundariesBuffer(buffer) ; 
    if(strcmp(name, sensors) == 0)      setSensorsBuffer(buffer) ; 

    if(strcmp(name, center_extent) == 0)   setCenterExtentBuffer(buffer) ; 
    if(strcmp(name, bbox) == 0)            setBBoxBuffer(buffer) ; 
    if(strcmp(name, transforms) == 0)      setTransformsBuffer(buffer) ; 
    if(strcmp(name, meshes) == 0)          setMeshesBuffer(buffer) ; 
    if(strcmp(name, nodeinfo) == 0)        setNodeInfoBuffer(buffer) ; 
}

void GMesh::setVertices(gfloat3* vertices)
{
    m_vertices = vertices ;
    m_vertices_buffer = new GBuffer( sizeof(gfloat3)*m_num_vertices, (void*)m_vertices, sizeof(gfloat3), 3 ) ;
    assert(sizeof(gfloat3) == sizeof(float)*3);
}
void GMesh::setVerticesBuffer(GBuffer* buffer)
{
    m_vertices_buffer = buffer ; 
    if(!buffer) return ; 

    m_vertices = (gfloat3*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    m_num_vertices = numBytes/sizeof(gfloat3);
}

void GMesh::explodeZVertices(float zoffset, float zcut)
{
    unsigned int noffset(0);
    for(unsigned int i=0 ; i < m_num_vertices ; i++)
    {
        gfloat3* v = m_vertices + i  ;
        if( v->z > zcut )
        {
            noffset += 1 ; 
            v->z += zoffset ; 
            printf("GMesh::explodeZVertices %6d : %10.4f %10.4f %10.4f \n", i, v->x, v->y, v->z );
        }
    }
    LOG(info) << "GMesh::explodeZVertices" 
              << " noffset " << noffset 
              << " zoffset " << zoffset 
              << " zcut " << zcut 
              ;
}




void GMesh::setNormals(gfloat3* normals)
{
    m_normals = normals ;
    m_normals_buffer = new GBuffer( sizeof(gfloat3)*m_num_vertices, (void*)m_normals, sizeof(gfloat3), 3 ) ;
    assert(sizeof(gfloat3) == sizeof(float)*3);
}
void GMesh::setNormalsBuffer(GBuffer* buffer)
{
    m_normals_buffer = buffer ; 
    if(!buffer) return ; 
    m_normals = (gfloat3*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    unsigned int num_normals = numBytes/sizeof(gfloat3);
    assert( m_num_vertices == num_normals );  // must load vertices before normals
}

void GMesh::setColors(gfloat3* colors)
{
    m_colors = colors ;
    m_colors_buffer = new GBuffer( sizeof(gfloat3)*m_num_vertices, (void*)m_colors, sizeof(gfloat3), 3  ) ;
    assert(sizeof(gfloat3) == sizeof(float)*3);
}
void GMesh::setColorsBuffer(GBuffer* buffer)
{
    m_colors_buffer = buffer ; 
    if(!buffer) return ; 
    
    m_colors = (gfloat3*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    unsigned int num_colors = numBytes/sizeof(gfloat3);

    assert( m_num_vertices == num_colors );  // must load vertices before colors
    //m_num_colors = m_num_vertices ; // TODO: get rid of m_num_colors: duplication is evil
}




// used from GMergedMesh
void GMesh::setCenterExtent(gfloat4* center_extent)  
{
    m_center_extent = center_extent ;  
    assert(m_num_solids > 0);
    m_center_extent_buffer = new GBuffer( sizeof(gfloat4)*m_num_solids, (void*)m_center_extent, sizeof(gfloat4), 4 ); 
    assert(sizeof(gfloat4) == sizeof(float)*4);
}
void GMesh::setCenterExtentBuffer(GBuffer* buffer) 
{
    m_center_extent_buffer = buffer ;  
    if(!buffer) return ; 

    m_center_extent = (gfloat4*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    m_num_solids = numBytes/sizeof(gfloat4) ;
}



void GMesh::setBBox(gbbox* bb)  
{
    m_bbox = bb ;  
    assert(m_num_solids > 0);
    m_bbox_buffer = new GBuffer( sizeof(gbbox)*m_num_solids, (void*)m_bbox, sizeof(gbbox), 6 ); 
    assert(sizeof(gbbox) == sizeof(float)*6);
}

void GMesh::setBBoxBuffer(GBuffer* buffer) 
{
    m_bbox_buffer = buffer ;  
    if(!buffer) return ; 

    m_bbox = (gbbox*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    unsigned int numSolids = numBytes/sizeof(gbbox) ;

    setNumSolids(numSolids);
}






void GMesh::setTransforms(float* transforms)  
{
    m_transforms = transforms ;  
    assert(m_num_solids > 0);

    unsigned int numElements = 16 ; 
    unsigned int size = sizeof(float)*numElements;

    LOG(info) << "GMesh::setTransforms " 
              << " num_solids " << m_num_solids 
              << " size " << size 
              << " fsize " << sizeof(float)
              ;

    m_transforms_buffer = new GBuffer( size*m_num_solids, (void*)m_transforms, size, numElements ); 
}


void GMesh::setTransformsBuffer(GBuffer* buffer) 
{
    m_transforms_buffer = buffer ;  
    if(!buffer) return ; 

    m_transforms = (float*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    unsigned int numElements = 16 ; 
    unsigned int size = sizeof(float)*numElements;
    unsigned int numSolids = numBytes/size ;
    //setNumSolids(numSolids);
}


unsigned int GMesh::getNumTransforms()
{
    return m_transforms_buffer ? m_transforms_buffer->getNumItems() : 0 ; 
}



void GMesh::setMeshes(unsigned int* meshes)  
{
    m_meshes = meshes ;  
    assert(m_num_solids > 0);
    unsigned int size = sizeof(unsigned int);
    m_meshes_buffer = new GBuffer( size*m_num_solids, (void*)m_meshes, size, 1 ); 
}

void GMesh::setMeshesBuffer(GBuffer* buffer) 
{
    m_meshes_buffer = buffer ;  
    if(!buffer) return ; 

    m_meshes = (unsigned int*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    unsigned int numElements = 1  ; 
    unsigned int size = sizeof(float)*numElements;
    unsigned int numSolids = numBytes/size ;
    setNumSolids(numSolids);
}



void GMesh::setNodeInfo(guint4* nodeinfo)  
{
    m_nodeinfo = nodeinfo ;  
    assert(m_num_solids > 0);
    unsigned int size = sizeof(guint4);
    assert(size == sizeof(unsigned int)*4 );
    m_nodeinfo_buffer = new GBuffer( size*m_num_solids, (void*)m_nodeinfo, size, 4 ); 
}
void GMesh::setNodeInfoBuffer(GBuffer* buffer) 
{
    m_nodeinfo_buffer = buffer ;  
    if(!buffer) return ; 

    m_nodeinfo = (guint4*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    unsigned int size = sizeof(guint4);
    assert(size == sizeof(unsigned int)*4 );
    unsigned int numSolids = numBytes/size ;
    setNumSolids(numSolids);
}








void GMesh::setNumSolids(unsigned int numSolids)
{
    if(m_num_solids == 0) 
    {
        m_num_solids = numSolids ; 
    }
    else
    {
        assert(numSolids == m_num_solids);
    }
}


unsigned int GMesh::findContainer(gfloat3 p)
{
   // find volumes that contains the point
   // returning the index of the volume with the smallest extent 
   // or 0 if none found
   //
    unsigned int container(0);
    float cext(FLT_MAX) ; 

    for(unsigned int index=0 ; index < m_num_solids ; index++)
    {
         gfloat4 ce = m_center_extent[index] ;
         gfloat3 hi(ce.x + ce.w, ce.y + ce.w, ce.z + ce.w );
         gfloat3 lo(ce.x - ce.w, ce.y - ce.w, ce.z - ce.w );

         if( 
              p.x > lo.x && p.x < hi.x  &&
              p.y > lo.y && p.y < hi.y  &&
              p.z > lo.z && p.z < hi.z 
           )
          {
               //printf("GMesh::findContainer %d   %10.4f %10.4f %10.4f %10.4f  \n", index, ce.x, ce.y, ce.z, ce.w  );
               if(ce.w < cext)
               {
                   cext = ce.w ; 
                   container = index ; 
               }
          }
    }
    return container ; 
}









void GMesh::setTexcoords(gfloat2* texcoords)
{
    if(!texcoords) return ;
    m_texcoords = texcoords ;
    m_texcoords_buffer = new GBuffer( sizeof(gfloat2)*m_num_vertices, (void*)m_texcoords, sizeof(gfloat2), 2  ) ;
    assert(sizeof(gfloat2) == sizeof(float)*2);
}
void GMesh::setTexcoordsBuffer(GBuffer* buffer)
{
    m_texcoords_buffer = buffer ; 
    if(!buffer) return ; 

    m_texcoords = (gfloat2*)buffer->getPointer(); 
    unsigned int numBytes = buffer->getNumBytes();
    unsigned int num_texcoords = numBytes/sizeof(gfloat2);
    assert( m_num_vertices == num_texcoords );  // must load vertices before texcoords
}







void GMesh::setFaces(guint3* faces)
{
    assert(sizeof(guint3) == 3*4);
    unsigned int totbytes = sizeof(guint3)*m_num_faces ;
    unsigned int itemsize = sizeof(guint3)/3 ;           // this regards the item as the individual index integer
    unsigned int nelem    = 1 ;                          // number of elements in the item

    m_faces = faces ;
    m_indices_buffer = new GBuffer( totbytes, (void*)m_faces, itemsize, nelem ) ;
    assert(sizeof(guint3) == sizeof(unsigned int)*3);
}

void GMesh::setIndicesBuffer(GBuffer* buffer)
{
    m_indices_buffer = buffer ; 
    if(!buffer) return ;

    m_faces = (guint3*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    m_num_faces = numBytes/sizeof(guint3);    // NB kludge equating "int" buffer to "unsigned int" 
}







void GMesh::setNodes(unsigned int* nodes)   // only makes sense to use from single subclasses instances like GMergedMesh 
{
    m_nodes = nodes ;
    m_nodes_buffer = new GBuffer( sizeof(unsigned int)*m_num_faces, (void*)m_nodes, sizeof(unsigned int), 1 ) ;
    assert(sizeof(unsigned int) == sizeof(unsigned int)*1);
}
void GMesh::setNodesBuffer(GBuffer* buffer)
{
    m_nodes_buffer = buffer ; 
    if(!buffer) return ;

    m_nodes = (unsigned int*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    unsigned int num_nodes = numBytes/sizeof(unsigned int);

    // assert(m_num_faces == num_nodes);   // must load indices before nodes
    if(m_num_faces != num_nodes)
        LOG(warning) << "GMesh::setNodesBuffer allowing inconsistency " ; 
}


void GMesh::setBoundaries(unsigned int* boundaries)
{
    m_boundaries = boundaries ;
    m_boundaries_buffer = new GBuffer( sizeof(unsigned int)*m_num_faces, (void*)m_boundaries, sizeof(unsigned int), 1 ) ;
    assert(sizeof(unsigned int) == sizeof(unsigned int)*1);
}
void GMesh::setBoundariesBuffer(GBuffer* buffer)
{
    m_boundaries_buffer = buffer ; 
    if(!buffer) return ;

    m_boundaries = (unsigned int*)buffer->getPointer();

    unsigned int numBytes = buffer->getNumBytes();
    unsigned int num_boundaries = numBytes/sizeof(unsigned int);

    // assert(m_num_faces == num_boundaries);   // must load indices before boundaries, for m_num_faces
    if(m_num_faces != num_boundaries)
        LOG(warning) << "GMesh::setBoundariesBuffer allowing inconsistency " ; 
}



void GMesh::setSensors(unsigned int* sensors)
{
    m_sensors = sensors ;
    m_sensors_buffer = new GBuffer( sizeof(unsigned int)*m_num_faces, (void*)m_sensors, sizeof(unsigned int), 1 ) ;
    assert(sizeof(unsigned int) == sizeof(unsigned int)*1);
}
void GMesh::setSensorsBuffer(GBuffer* buffer)
{
    m_sensors_buffer = buffer ; 
    if(!buffer) return ;

    m_sensors = (unsigned int*)buffer->getPointer();

    unsigned int numBytes = buffer->getNumBytes();
    unsigned int num_sensors = numBytes/sizeof(unsigned int);


    //assert(m_num_faces == num_sensors);   // must load indices before sensors, for m_num_faces
    if(m_num_faces != num_sensors)
        LOG(warning) << "GMesh::setSensorsBuffer allowing inconsistency " ; 

}




void GMesh::setColor(float r, float g, float b)
{
    //assert(m_num_colors == m_num_vertices);
    if(!m_colors)
    {
        setColors(new gfloat3[m_num_vertices]);
    }
    for(unsigned int i=0 ; i<m_num_vertices ; ++i )
    {
        m_colors[i].x  = r ;
        m_colors[i].y  = g ;
        m_colors[i].z  = b ;
    }
}



void GMesh::Dump(const char* msg, unsigned int nmax)
{
    printf("%s\n", msg);
    for(unsigned int i=0 ; i < std::min(nmax,m_num_vertices) ; i++)
    {
        gfloat3& vtx = m_vertices[i] ;
        printf(" vtx %5u  %10.3f %10.3f %10.3f \n", i, vtx.x, vtx.y, vtx.z );
    } 

    for(unsigned int i=0 ; i < std::min(nmax,m_num_vertices) ; i++)
    {
        gfloat3& nrm = m_normals[i] ;
        printf(" nrm %5u  %10.3f %10.3f %10.3f \n", i, nrm.x, nrm.y, nrm.z );
    } 


    if(hasTexcoords())
    { 
        for(unsigned int i=0 ; i < std::min(nmax,m_num_vertices) ; i++)
        {
            gfloat2& tex = m_texcoords[i] ;
            printf(" tex %5u  %10.3f %10.3f  \n", i, tex.u, tex.v );
        } 
    }

    for(unsigned int i=0 ; i < std::min(nmax,m_num_vertices) ; i++)
    {
        gfloat3& col = m_colors[i] ;
        printf(" col %5u  %10.3f %10.3f %10.3f \n", i, col.x, col.y, col.z );
    } 


    for(unsigned int i=0 ; i < std::min(nmax,m_num_faces) ; i++)
    {
        guint3& fac = m_faces[i] ;
        printf(" fac %5u  %5u %5u %5u \n", i, fac.x, fac.y, fac.z );
    } 

    for(unsigned int i=0 ; i < std::min(nmax,m_num_faces) ; i++)
    {
        unsigned int& node = m_nodes[i] ;
        unsigned int& boundary = m_boundaries[i] ;
        printf(" fac %5u  node %5u boundary %5u  \n", i, node, boundary );
    } 
}


void GMesh::Summary(const char* msg)
{
   printf("%s idx %u vx %u fc %u n %s sn %s \n",
      msg, 
      m_index, 
      m_num_vertices, 
      m_num_faces,
      m_name,
      m_shortname
   );

   if(m_low) printf("%10s %10.3f %10.3f %10.3f\n",
         "low",
         m_low->x,
         m_low->y,
         m_low->z);

   if(m_high) printf("%10s %10.3f %10.3f %10.3f\n", 
          "high",
          m_high->x,
          m_high->y,
          m_high->z);

   if(m_dimensions) printf("%10s %10.3f %10.3f %10.3f extent %10.3f\n", 
          "dimen",
          m_dimensions->x,
          m_dimensions->y,
          m_dimensions->z,
          m_extent);

   if(m_center) printf("%10s %10.3f %10.3f %10.3f\n", 
          "center",
          m_center->x,
          m_center->y,
          m_center->z);

   if(m_center_extent) printf("%10s %10.3f %10.3f %10.3f %10.3f \n", 
          "center_extent",
          m_center_extent->x,
          m_center_extent->y,
          m_center_extent->z,
          m_center_extent->w);

   m_model_to_world->Summary(msg);
}



gbbox GMesh::findBBox(gfloat3* vertices, unsigned int num_vertices)
{
    gbbox bb(gfloat3(FLT_MAX), gfloat3(-FLT_MAX)) ; 

    for( unsigned int i = 0; i < num_vertices ;++i )
    {
        gfloat3& v = vertices[i];

        bb.min.x = std::min( bb.min.x, v.x);
        bb.min.y = std::min( bb.min.y, v.y);
        bb.min.z = std::min( bb.min.z, v.z);

        bb.max.x = std::max( bb.max.x, v.x);
        bb.max.y = std::max( bb.max.y, v.y);
        bb.max.z = std::max( bb.max.z, v.z);
    }
    return bb ; 
} 


gfloat4 GMesh::findCenterExtentDeprecated(gfloat3* vertices, unsigned int num_vertices)
{
    gfloat3  low( 1e10f, 1e10f, 1e10f);
    gfloat3 high( -1e10f, -1e10f, -1e10f);

    for( unsigned int i = 0; i < num_vertices ;++i )
    {
        gfloat3& v = vertices[i];

        low.x = std::min( low.x, v.x);
        low.y = std::min( low.y, v.y);
        low.z = std::min( low.z, v.z);

        high.x = std::max( high.x, v.x);
        high.y = std::max( high.y, v.y);
        high.z = std::max( high.z, v.z);
    }

    gfloat3 dimensions(high.x - low.x, high.y - low.y, high.z - low.z );
    float extent = 0.f ;
    extent = std::max( dimensions.x , extent );
    extent = std::max( dimensions.y , extent );
    extent = std::max( dimensions.z , extent );
    extent = extent / 2.0f ;         
 
    gfloat4 center_extent((high.x + low.x)/2.0f, (high.y + low.y)/2.0f , (high.z + low.z)/2.0f, extent );

    return center_extent ; 
}



void GMesh::updateBounds()
{
    gbbox   bb = findBBox(m_vertices, m_num_vertices);
    gfloat4 ce = bb.center_extent() ;

    m_model_to_world = new GMatrix<float>( ce.x, ce.y, ce.z, ce.w );

    //
    // extent is half the maximal dimension 
    // 
    // model coordinates definition
    //      all vertices are contained within model coordinates box  (-1:1,-1:1,-1:1) 
    //      model coordinates origin (0,0,0) corresponds to world coordinates  m_center
    //  
    // world -> model
    //        * translate by -m_center 
    //        * scale by 1/m_extent
    //
    //  model -> world
    //        * scale by m_extent
    //        * translate by m_center
    //


    if(m_bbox == NULL)
    {
        m_bbox = new gbbox(bb) ;
    }
    else
    {
        m_bbox[0].min = bb.min ;
        m_bbox[0].max = bb.max ;
    }


    if(m_center_extent == NULL)
    {
        m_center_extent = new gfloat4( ce.x, ce.y, ce.z, ce.w );
    }
    else
    {
        // avoid stomping on position of array of center_extent in case of MergedMesh, 
        // instead just overwrite solid 0 

        m_center_extent[0].x = ce.x ;
        m_center_extent[0].y = ce.y ;
        m_center_extent[0].z = ce.z ;
        m_center_extent[0].w = ce.w ;
    }



}



void GMesh::updateBounds(gfloat3& low, gfloat3& high, GMatrixF& transform)
{
    if(m_low && m_high)
    {   
        gfloat3 mlow(*m_low) ; 
        gfloat3 mhigh(*m_high) ; 

        mlow  *= transform ; 
        mhigh *= transform ; 

        low.x = std::min( low.x, mlow.x);
        low.y = std::min( low.y, mlow.y);
        low.z = std::min( low.z, mlow.z);

        high.x = std::max( high.x, mhigh.x);
        high.y = std::max( high.y, mhigh.y);
        high.z = std::max( high.z, mhigh.z);
   }
}



gfloat3* GMesh::getTransformedVertices(GMatrixF& transform )
{
     gfloat3* vertices = new gfloat3[m_num_vertices];
     for(unsigned int i = 0; i < m_num_vertices; i++)
     {  
         vertices[i].x = m_vertices[i].x ;   
         vertices[i].y = m_vertices[i].y ;   
         vertices[i].z = m_vertices[i].z ;   

         vertices[i] *= transform ;
     }   
     return vertices ;
}

gfloat3* GMesh::getTransformedNormals(GMatrixF& transform )
{
     gfloat3* normals = new gfloat3[m_num_vertices];
     for(unsigned int i = 0; i < m_num_vertices; i++)
     {  
         gfloat4 nrm(m_normals[i], 0.);   // w=0 as direction, not position 

         nrm *= transform ; 
         // NB should be transpose of inverse, 
         // (so only OK if orthogonal, that means rotations only no non-uniform scaling)

         normals[i].x = nrm.x ; 
         normals[i].y = nrm.y ; 
         normals[i].z = nrm.z ; 

     }   
     return normals ;
}



void GMesh::updateDistinctBoundaries()
{
    for(unsigned int i=0 ; i < getNumFaces() ; i++)
    {
        unsigned int index = m_boundaries[i] ;
        if(std::count(m_distinct_boundaries.begin(), m_distinct_boundaries.end(), index ) == 0) m_distinct_boundaries.push_back(index);
    }  
    std::sort( m_distinct_boundaries.begin(), m_distinct_boundaries.end() );
}
 
std::vector<unsigned int>& GMesh::getDistinctBoundaries()
{
    if(m_distinct_boundaries.size()==0) updateDistinctBoundaries();
    return m_distinct_boundaries ;
}


bool GMesh::isFloatBuffer(const char* name)
{

    return ( strcmp( name, vertices) == 0 || 
             strcmp( name, normals) == 0  || 
             strcmp( name, center_extent ) == 0  || 
             strcmp( name, bbox ) == 0  || 
          //   strcmp( name, wavelength ) == 0  || 
          //   strcmp( name, reemission ) == 0  || 
             strcmp( name, transforms ) == 0  || 
             strcmp( name, colors) == 0 );
}

bool GMesh::isIntBuffer(const char* name)
{
    return ( 
             strcmp( name, indices) == 0     || 
             strcmp( name, nodes) == 0       || 
             strcmp( name, sensors) == 0     || 
             strcmp( name, boundaries ) == 0 
          );
}
bool GMesh::isUIntBuffer(const char* name)
{
    return 
           ( 
              strcmp( name, nodeinfo) == 0  ||
              strcmp( name, meshes) == 0  
           );
}

void GMesh::saveBuffer(const char* path, const char* name, GBuffer* buffer)
{
    LOG(debug) << "GMesh::saveBuffer "
               << " name " << std::setw(25) << name 
               << " path " << path  
               ;

    if(isFloatBuffer(name))     buffer->save<float>(path);
    else if(isIntBuffer(name))  buffer->save<int>(path);
    else if(isUIntBuffer(name)) buffer->save<unsigned int>(path);
    else 
       printf("GMesh::saveBuffer WARNING NOT saving uncharacterized buffer %s into %s \n", name, path );
}


void GMesh::loadBuffer(const char* path, const char* name)
{
    GBuffer* buffer(NULL); 
    if(isFloatBuffer(name))                    buffer = GBuffer::load<float>(path);
    else if(isIntBuffer(name))                 buffer = GBuffer::load<int>(path);
    else if(isUIntBuffer(name))                buffer = GBuffer::load<unsigned int>(path);
    else
        printf("GMesh::loadBuffer WARNING not loading %s from %s \n", name, path ); 

    if(buffer) setBuffer(name, buffer);
}

std::vector<std::string>& GMesh::getNames()
{
    return m_names ; 
}

std::string GMesh::getVersionedBufferName(std::string& name)
{
    std::string vname = name ;
    if(m_version)
    {
        if(vname.compare("vertices") == 0 || 
           vname.compare("indices") == 0  || 
           vname.compare("colors") == 0  || 
           vname.compare("normals") == 0)
           { 
               vname += m_version ;
               LOG(warning) << "GMesh::loadBuffers version setting changed buffer name to " << vname ; 
           }
    }
    return vname ; 
}



GMesh* GMesh::load(const char* dir, const char* typedir, const char* instancedir)
{
    fs::path cachedir(dir);
    if(typedir)     cachedir /= typedir ;
    if(instancedir) cachedir /= instancedir ;

    GMesh* mesh(NULL);
    if(!fs::exists(cachedir))
    {
        printf("GMesh::load directory %s DOES NOT EXIST \n", dir);
    }
    else
    {
        mesh = new GMesh(0, NULL, 0, NULL, 0, NULL, NULL );
        mesh->loadBuffers(cachedir.string().c_str());
    }
    return mesh ; 
}


void GMesh::save(const char* dir, const char* typedir, const char* instancedir)
{
    fs::path cachedir(dir);
    if(typedir)     cachedir /= typedir ;
    if(instancedir) cachedir /= instancedir ;

    if(!fs::exists(cachedir))
    {
        if (fs::create_directories(cachedir))
        {
            printf("GMesh::save created directory %s \n", cachedir.string().c_str() );
        }
    }

    if(fs::exists(cachedir) && fs::is_directory(cachedir))
    {
        saveBuffers(cachedir.string().c_str());
    }
    else
    {
        printf("GMesh::save directory %s DOES NOT EXIST \n", dir);
    }
}


void GMesh::loadBuffers(const char* dir)
{
    for(unsigned int i=0 ; i<m_names.size() ; i++)
    {
        std::string name = m_names[i];
        std::string vname = getVersionedBufferName(name);  
        fs::path bufpath(dir);
        bufpath /= vname + ".npy" ; 

        if(fs::exists(bufpath) && fs::is_regular_file(bufpath))
        { 
            loadBuffer(bufpath.string().c_str(), name.c_str());
        }
    } 
    updateBounds();
}


void GMesh::saveBuffers(const char* dir)
{
    for(unsigned int i=0 ; i<m_names.size() ; i++)
    {
        std::string name = m_names[i];
        std::string vname = getVersionedBufferName(name);  
        fs::path bufpath(dir);
        bufpath /= vname + ".npy" ; 

        GBuffer* buffer = getBuffer(name.c_str());
        if(!buffer) continue ; 

        saveBuffer(bufpath.string().c_str(), name.c_str(), buffer);  
    } 
}


GMesh* GMesh::makeDedupedCopy()
{
    GMeshFixer fixer(this);
    fixer.copyWithoutVertexDuplicates();   
    return fixer.getDst(); 
}


GMesh* GMesh::load_deduped(const char* dir, const char* typedir, const char* instancedir)
{
    GMesh* gm = GMesh::load(dir, typedir, instancedir) ;
    GMesh* dm = gm->makeDedupedCopy();
    delete gm ; 
    return dm ; 
}

void GMesh::findShortName()
{
   if(!m_name) return ; 
   m_shortname = trimPointerSuffixPrefix(m_name, NULL );   
}

