/*
 * Copyright (c) 2019 Opticks Team. All Rights Reserved.
 *
 * This file is part of Opticks
 * (see https://bitbucket.org/simoncblyth/opticks).
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License.  
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 */

#include <csignal>
#include <cstdio>
#include <climits>
#include <cstring>

#include "SPack.hh"

// npy-
#include "NGLM.hpp"
#ifdef OLD_SENSOR
#include "NSensor.hpp"
#endif

#include "GPropertyMap.hh"
#include "GMesh.hh"

#include "GMatrix.hh"
#include "GBndLib.hh"
#include "GSurfaceLib.hh"

#include "GVolume.hh"

#include "PLOG.hh"
#include "GGEO_BODY.hh"


GVolume::GVolume( unsigned index, GMatrix<float>* transform, const GMesh* mesh, void* origin_node )
    : 
    GNode(index, transform, mesh ),
    m_boundary(-1),
    m_csgflag(CSG_PARTLIST),
    m_csgskip(false),
#ifdef OLD_SENSOR
    m_sensor(NULL),
    m_sensor_surface_index(0),
#endif
    m_sensor_index(-1),
    m_pvname(NULL),
    m_lvname(NULL),
    m_parts(NULL),
    m_pt(NULL),
    m_parallel_node(NULL), 
    m_copyNumber(-1),
    m_origin_node(origin_node),
    m_outer_volume(NULL)
{
}

OpticksCSG_t GVolume::getCSGFlag() const 
{
    return m_csgflag ; 
}

bool GVolume::isCSGSkip() const 
{
    return m_csgskip ; 
}
void GVolume::setCSGSkip(bool csgskip)
{
    m_csgskip = csgskip ; 
}


unsigned GVolume::getBoundary() const 
{
    return m_boundary ; 
}

void GVolume::setCopyNumber(unsigned copyNumber)
{
    m_copyNumber = copyNumber ; 
}
unsigned GVolume::getCopyNumber() const 
{
    return m_copyNumber ; 
}

void GVolume::setPVName(const char* pvname)
{
    m_pvname = strdup(pvname);
}
void GVolume::setLVName(const char* lvname)
{
    m_lvname = strdup(lvname);
}

const char* GVolume::getPVName() const 
{
    return m_pvname ; 
}
const char* GVolume::getLVName() const 
{
    return m_lvname ; 
}


void GVolume::Summary(const char* msg )
{
    if(!msg) msg = getDescription();
    if(!msg) msg = "GVolume::Summary" ;
    printf("%s\n", msg );
}

std::string GVolume::description() const 
{
    const char* desc_ = getDescription() ;

    std::string desc ;
    if(desc_) desc.assign(desc_);
  
    return desc; 
}


GParts* GVolume::getParts() const 
{
    return m_parts ;  
}
void GVolume::setParts(GParts* pts)
{
    m_parts = pts ; 
}


GPt* GVolume::getPt() const 
{
    return m_pt ;  
}
void GVolume::setPt(GPt* pt)
{
    m_pt = pt ; 
}






// ancillary slot for a parallel node tree, used by X4PhysicalVolume
void* GVolume::getParallelNode() const 
{
    return m_parallel_node ; 
}
void GVolume::setParallelNode(void* pnode)
{
    m_parallel_node = pnode ; 
}
 

/**
GVolume::getOriginNode
------------------------

*OriginNode* set in ctor is used to record the G4VPhysicalVolume from whence the GVolume 
was converted, see X4PhysicalVolume::convertNode

**/

void* GVolume::getOriginNode() const 
{
    return m_origin_node ; 
}


/**
GVolume::setOuterVolume
-------------------------

Used from the recursive GInstancer::labelRepeats_r enabling all volumes 
within the repeated instances to know their base volumes.

**/

void GVolume::setOuterVolume(const GVolume* outer_volume) 
{
    m_outer_volume = outer_volume ; 
}
const GVolume* GVolume::getOuterVolume() const 
{
   return m_outer_volume ;  
}







void GVolume::setCSGFlag(OpticksCSG_t csgflag)
{
    m_csgflag = csgflag ; 
}

void GVolume::setBoundary(unsigned boundary)
{
    m_boundary = boundary ; 
    setBoundaryIndices( boundary );
}


void GVolume::setBoundaryAll(unsigned boundary)
{
    unsigned nchild = getNumChildren();
    if(nchild > 0)
    {
        for(unsigned i=0 ; i < nchild ; i++)
        {
            GNode* node = getChild(i);
            GVolume* sub = dynamic_cast<GVolume*>(node);
            sub->setBoundary(boundary);
        }
    } 
}





//unsigned GVolume::getIdentityIndex() const 
//{
//    unsigned identity_index = m_copyNumber  ;  // // SHOULD BE m_sensor_index ? which will match m_copyNumber for JUNO  
//    return identity_index ; 
//}


/**
GVolume::getIdentity
----------------------

The volume identity quad is available GPU side for all intersects
with geometry.

1. node_index (3 bytes at least as JUNO needs more than 2-bytes : so little to gain from packing) 
2. triplet_identity (4 bytes, pre-packed)
3. SPack::Encode22(mesh_index, boundary_index)

   * mesh_index: 2 bytes easily enough, 0xffff = 65535
   * boundary_index: 2 bytes easily enough  

4. sensor_index (2 bytes easily enough) 

The sensor_identifier is detector specific so would have to allow 4-bytes 
hence exclude it from this identity, instead can use sensor_index to 
look up sensor_identifier within G4Opticks::getHit 

Formerly::

   guint4 id(getIndex(), getMeshIndex(),  getBoundary(), getSensorIndex()) ;

**/

glm::uvec4 GVolume::getIdentity() const 
{
    unsigned index = getIndex() ;
    unsigned tripid = getTripletIdentity();
    if(index > 0 ) assert( tripid != 0 ); 
    glm::uvec4 id(index, tripid, getShapeIdentity(), getSensorIndex()) ; 
    return id ; 
}

/**
GVolumne::getShapeIdentity
----------------------------

The shape identity packs mesh index and boundary index together.
This info is used GPU side by::

   oxrap/cu/material1_propagate.cu:closest_hit_propagate

**/

unsigned GVolume::getShapeIdentity() const
{
    return SPack::Encode22( getMeshIndex(), getBoundary() ); 
}



/**
GVolume::getNodeInfo
----------------------

Used from GMergedMesh::mergeVolumeIdentity

**/

glm::uvec4 GVolume::getNodeInfo() const
{
    const GMesh* mesh = getMesh();
    unsigned nvert = mesh->getNumVertices();
    unsigned nface = mesh->getNumFaces();
    unsigned nodeIndex = getIndex();
    const GNode* parent = getParent();
    unsigned parentIndex = parent ? parent->getIndex() : UINT_MAX ;

    glm::uvec4 nodeinfo(nface, nvert, nodeIndex, parentIndex); 
    return nodeinfo ;  
}



/**
GVolume::setSensorIndex
-------------------------

sensorIndex is expected to be a 0-based contiguous index, with the 
default value of -1 meaning no sensor.

This is canonically invoked from X4PhysicalVolume::convertNode during GVolume creation.

* GNode::setSensorIndices duplicates the index to all faces of m_mesh triangulated geometry

**/
void GVolume::setSensorIndex(int sensor_index)
{
    m_sensor_index = sensor_index ; 
    setSensorIndices( m_sensor_index );   
}
int GVolume::getSensorIndex() const 
{
    return m_sensor_index ;      
}
bool GVolume::hasSensorIndex() const
{
    return m_sensor_index > -1 ; 
}

#ifdef OLD_SENSOR
void GVolume::setSensor(NSensor* sensor)
{
    m_sensor = sensor ; 
    // every triangle needs a value... use 0 to mean unset, so sensor   
    setSensorIndices( NSensor::RefIndex(sensor) );
}
NSensor* GVolume::getSensor()
{
    return m_sensor ; 
}
void GVolume::setSensorSurfaceIndex(unsigned int ssi)
{
    m_sensor_surface_index = ssi ; 
}
unsigned int GVolume::getSensorSurfaceIndex()
{
    return m_sensor_surface_index ; 
}
#endif



void GVolume::Dump( const std::vector<GVolume*>& volumes, const char* msg )
{
    unsigned numVolume = volumes.size() ;
    LOG(info) << msg << " numVolume " << numVolume ; 
    for(unsigned i=0 ; i < numVolume ; i++) volumes[i]->dump(); 
}

