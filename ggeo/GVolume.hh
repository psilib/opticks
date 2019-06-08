#pragma once

#include <vector>
#include <string>

class GMesh ;
class GParts ; 
class NSensor ; 
template <typename T> class GMatrix ; 

#include "OpticksCSG.h"
//#include "GVector.hh"
#include "GNode.hh"

#include "GGEO_API_EXPORT.hh"
#include "GGEO_HEAD.hh"

/**
GVolume
========

GVolume was formerly mis-named as "GSolid", which was always a confusing name.

Constituent "solids" (as in CSG, and G4VSolid in G4) are mesh-level-objects 
(relatively few in a geometry, corresponding to each distinct shape)

Whereas GVolume(GNode) are node-level-objects (relatively many in the geometry)
which refer to the corresponding mesh-level-objects by pointer or index.

**/

class GGEO_API GVolume : public GNode {
  public:
      static void Dump( const std::vector<GVolume*>& solids, const char* msg="GVolume::Dump" );
  public:
      GVolume( unsigned index, GMatrix<float>* transform, const GMesh* mesh );
  public:
      void     setCSGFlag(OpticksCSG_t flag);
      void     setCSGSkip(bool csgskip);
      void     setBoundary(unsigned boundary);     // also sets BoundaryIndices array
      void     setBoundaryAll(unsigned boundary);  // recursive over tree
      void     setSensor(NSensor* sensor);         // also sets SensorIndices
      void     setSensorSurfaceIndex(unsigned int ssi);
      unsigned getSensorSurfaceIndex();
  public:
      void        setPVName(const char* pvname);
      void        setLVName(const char* lvname);
      const char* getPVName() const ;
      const char* getLVName() const ;
  public:
      OpticksCSG_t getCSGFlag();
      bool         isCSGSkip();
      unsigned     getBoundary() const ;
      guint4       getIdentity();
      NSensor*     getSensor();
  public:
      GParts*      getParts();
      void         setParts(GParts* pts);
  public:
      // ancillary slot for a parallel node tree, used by X4PhysicalVolume
      void*        getParallelNode() const ;
      void         setParallelNode(void* pnode); 
  public: 
      void Summary(const char* msg="GVolume::Summary");
      std::string description();
  private:
      int               m_boundary ; 
      OpticksCSG_t      m_csgflag ; 
      bool              m_csgskip ; 
      NSensor*          m_sensor ; 
      const char*       m_pvname ; 
      const char*       m_lvname ; 
      unsigned int      m_sensor_surface_index ; 
      GParts*           m_parts ; 
      void*             m_parallel_node ; 

};
#include "GGEO_TAIL.hh"

