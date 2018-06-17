#include <cassert>
#include <sstream>
#include <iomanip>

#include "PLOG.hh"
#include "BStr.hh"
#include "NGLM.hpp"
#include "YOG.hh"

namespace YOG {

std::string Mt::desc() const 
{
    std::stringstream ss ; 
    ss
        << "Mt "
        << std::setw(30) << name
        ; 
    return ss.str();
}

std::string Mh::desc() const 
{
    std::stringstream ss ; 
    ss
        << "Mh "
        << std::setw(4) << lvIdx 
        << " : "
        << std::setw(30) << soName
        << " "
        << lvName 
        ; 
    return ss.str();
}

std::string Nd::desc() const 
{
    std::stringstream ss ; 
    ss
        << "Nd "
        << " ndIdx:" << std::setw(4) << ndIdx 
        << " soIdx:" << std::setw(4) << soIdx 
        << " nch:"   << std::setw(4) << children.size()
        << " par:"   << std::setw(4) << parent  
        << " xf:"  << ( transform ? glm::to_string(*transform) : "-" )
        ; 
    return ss.str();
}

Sc::Sc(int root_)
   :
   root(root_)
{
}

std::string Sc::desc() const 
{
    std::stringstream ss ; 
    ss
        << "Sc "
        << " nodes:" << std::setw(4) << nodes.size()
        << " meshes:" << std::setw(4) << meshes.size()
        ; 
    return ss.str();
}

int Sc::lv2so(int lvIdx) const   // find local mesh index from the external lvIdx 
{
    int index(-1); 
    unsigned count(0); 
    for(int i=0 ; i < meshes.size() ; i++)
    {
       const Mh* mh = meshes[i] ; 
       if(mh->lvIdx == lvIdx ) 
       {
           index = i ; 
           count++ ; 
       } 
    }
    assert( count == 1);
    return index ;  
}

bool Sc::has_mesh(int lvIdx) const 
{
    unsigned count(0); 
    for(int i=0 ; i < meshes.size() ; i++)
    {
       const Mh* mh = meshes[i] ; 
       if(mh->lvIdx == lvIdx ) count++ ; 
    }
    return count == 1 ; 
}

/**
add_mesh
---------

* only adds if no mesh with lvIdx is present already 


**/

int Sc::add_mesh(
                 int lvIdx,
                 int mtIdx,
                 const std::string& lvName, 
                 const std::string& soName)
{
    int soIdx = -1 ; 
    if(!has_mesh(lvIdx))
    {
        soIdx = meshes.size(); 
        meshes.push_back(new Mh { lvIdx, mtIdx, lvName, soName, soIdx, NULL, NULL, NULL, NULL }) ;
    }
    int soIdx2 = lv2so(lvIdx);
    if(soIdx > -1 ) assert( soIdx2 == soIdx ) ; // when a new mesh is added, can check local indices match
    return soIdx2 ; 
}


int Sc::get_material_idx(const std::string& matName) const 
{
    int idx = -1 ; 
    unsigned count(0); 
    for(int i=0 ; i < materials.size() ; i++)
    {
        const Mt* mt = materials[i];
        if(strcmp(mt->name.c_str(), matName.c_str()) == 0) 
        {
            idx = i ; 
            count++ ; 
        }
    }
    assert( count < 2 ); 
    return idx ; 
}


int Sc::add_material(const std::string& matName)
{
    int idx = get_material_idx(matName) ;
    if( idx == -1 )
    {
        idx = materials.size();
        materials.push_back(new Mt { matName }); 
    }     
    return idx ; 
}


int Sc::add_node(int lvIdx, 
                 int mtIdx,
                 const std::string& lvName, 
                 const std::string& pvName, 
                 const std::string& soName, 
                 const glm::mat4* transform, 
                 const std::string& boundary,
                 int depth, 
                 bool selected)
{

     int soIdx = add_mesh( lvIdx, mtIdx, lvName, soName);  

     assert( soIdx > -1 );  

     // soIdx is zero-based local index, lvIdx is an externally imposed index

     int ndIdx = nodes.size() ;
     int parent = -1 ; 
     Nd* nd = new Nd {ndIdx, soIdx, transform, boundary, pvName, depth, this, selected, parent }  ;

     LOG(info) << nd->desc(); 

     nodes.push_back(nd) ;
     return ndIdx ; 
}


Nd* Sc::get_node(int nodeIdx) const 
{
    assert( nodeIdx < nodes.size() );
    return nodes[nodeIdx] ;  
}

Mh* Sc::get_mesh_for_node(int nodeIdx) const 
{
    Nd* nd = get_node(nodeIdx) ; 
    Mh* mh = meshes[nd->soIdx];  
    assert( mh );
    assert( mh->soIdx == nd->soIdx );
    return mh ;  
}




int Sc::add_test_node(int lvIdx)
{
    int mtIdx = lvIdx ; 
    std::string lvName = BStr::concat<int>("lv", lvIdx, NULL) ;   
    std::string pvName = BStr::concat<int>("pv", lvIdx, NULL) ;   
    std::string soName = BStr::concat<int>("so", lvIdx, NULL) ;   
    const glm::mat4* transform = new glm::mat4 ; 
    std::string boundary = BStr::concat<int>("bd", lvIdx, NULL) ;   
    int depth = 0 ; 
    bool selected = true ;  

    int ndIdx = add_node(lvIdx, 
                         mtIdx,
                         lvName, 
                         pvName, 
                         soName, 
                         transform, 
                         boundary,
                         depth, 
                         selected);  

    return ndIdx ; 
}

} // namespace
