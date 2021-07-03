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

#include <string>
#include "G4Material.hh"
#include "X4PhysicsVector.hh"
#include "X4Material.hh"
#include "X4MaterialPropertiesTable.hh"
#include "GMaterial.hh"

#include "BFile.hh"
#include "SDigest.hh"
#include "PLOG.hh"


const plog::Severity X4Material::LEVEL = debug ; 


std::string X4Material::Digest()
{
    const G4MaterialTable* mtab  = G4Material::GetMaterialTable();
    const std::vector<G4Material*>& materials = *mtab ; 
    return Digest(materials);  
}

std::string X4Material::Digest( const std::vector<G4Material*>& materials )
{
    SDigest dig ;
    for(unsigned i=0 ; i < materials.size() ; i++)
    {
        const G4Material* material = materials[i] ; 
        std::string idig = Digest(material);
        dig.update( const_cast<char*>(idig.data()), idig.size() );  
    } 
    return dig.finalize();
}

std::string X4Material::Digest( const G4Material* material )
{
    if(!material) return "" ; 
    G4MaterialPropertiesTable* mpt = material->GetMaterialPropertiesTable() ; 
    const G4String& name = material->GetName();    
    std::string dmpt = X4MaterialPropertiesTable::Digest(mpt) ; 
    SDigest dig ;
    dig.update( const_cast<char*>(name.data()), name.size() );  
    dig.update( const_cast<char*>(dmpt.data()), dmpt.size() );  
    return dig.finalize();
}


GMaterial* X4Material::Convert( const G4Material* material, char mode )
{
    X4Material xmat(material, mode);
    GMaterial* mat = xmat.getMaterial(); 
    return mat ; 
}




GMaterial* X4Material::getMaterial()
{
    return m_mat ; 
}

X4Material::X4Material( const G4Material* material, char mode ) 
    :
    m_material(material),
    m_mode(mode),
    m_mpt(material->GetMaterialPropertiesTable()),
    m_has_efficiency(m_mpt ? HasEfficiencyProperty(m_mpt) : false),
    m_mat(NULL)
{
    init() ;
}

bool X4Material::HasEfficiencyProperty(const G4MaterialPropertiesTable* mpt_) // static
{
    G4MaterialPropertiesTable* mpt = const_cast<G4MaterialPropertiesTable*>(mpt_) ; // G4 not const-correct 

    const char* key = "EFFICIENCY" ; 
    G4MaterialPropertyVector* mpv = mpt->GetProperty(key) ; 

    bool ret = false ; 
    if(mpv) 
    {
         LOG(LEVEL) << "found mpv for " << key ; 
         ret = true ; 
    }   
    else if( mpt->ConstPropertyExists(key) )
    {
         LOG(LEVEL) << "found ConstProperty  " << key ; 
         ret = true ; 
    }
    else 
    {
    }
    return ret ; 
}


/**
X4Material::init
-------------------

FORMERLY set the index on collecting into GMaterialLib, 
now are just passing the creation index along  

**/
void X4Material::init()
{
    const std::string& matname_ = m_material->GetName() ;
    const char* matname = matname_.c_str();

    std::string name = BFile::Name( matname ); 
    unsigned index = m_material->GetIndex() ;

    m_mat = new GMaterial(name.c_str(), index) ; 
    if( m_mpt )
    {
        X4MaterialPropertiesTable::Convert( m_mat, m_mpt, m_mode );
    }
}


size_t X4Material::NumProp(G4MaterialPropertiesTable* mpt)
{
    std::vector<G4String> propNames = mpt->GetMaterialPropertyNames() ; 
    size_t num_prop = 0 ; 
    for(size_t j=0 ; j < propNames.size()  ; j++)
    {
        const G4String& name = propNames[j]; 
        G4MaterialPropertyVector* prop = mpt->GetProperty(name.c_str());  
        if( prop != nullptr )  num_prop += 1 ; 
    }
    return num_prop ; 
}


std::string X4Material::DescProps(G4MaterialPropertiesTable* mpt, int wid)
{
    size_t num_prop = NumProp(mpt); 
    std::stringstream ss ; 
    ss << " num_prop " << std::setw(3) << num_prop ; 
    std::vector<G4String> propNames = mpt->GetMaterialPropertyNames() ; 
    for(size_t j=0 ; j < propNames.size()  ; j++)
    {
        const G4String& name = propNames[j]; 
        G4MaterialPropertyVector* prop = mpt->GetProperty(name.c_str());  
        if( prop != nullptr ) ss << " " << std::setw(wid) << name << " " ;  
    }
    std::string str = ss.str(); 
    return str ; 
}


std::string X4Material::Desc(const std::vector<G4Material*>& mtlist)
{
    size_t num_mt = mtlist.size() ; 

    std::stringstream ss ; 
    ss << " num_mt " <<  num_mt << std::endl ; 

    for(size_t i=0 ; i < num_mt ; i++)
    {
        const G4Material* mt = mtlist[i];
        G4MaterialPropertiesTable* mpt = mt->GetMaterialPropertiesTable() ; 
        ss  
            << std::setw(4) << i  
            << " : "
            << std::setw(30) << mt->GetName()
            << " : "
            << ( mpt == nullptr ? "" : DescProps(mpt, 20) ) 
            << std::endl
            ; 
    }
    std::string str = ss.str(); 
    return str ; 
}



