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


#include "NGLM.hpp"
#include "NGLMExt.hpp"
#include "GLMFormat.hpp"

//#define GLMVEC4(g) glm::vec4((g).x,(g).y,(g).z,(g).w) 

#include "GGeo.hh"

#include "Composition.hh"
#include "Opticks.hh"
#include "OpticksHub.hh"
#include "OpticksEvent.hh"
#include "OpticksAim.hh"

#include "PLOG.hh"


const plog::Severity OpticksAim::LEVEL = PLOG::EnvLevel("OpticksAim", "DEBUG") ; 


OpticksAim::OpticksAim(OpticksHub* hub) 
    :
    m_ok(hub->getOpticks()),
    m_dbgaim(m_ok->isDbgAim()),   // --dbgaim
    m_hub(hub),
    m_composition(hub->getComposition()),
    m_ggeo(NULL),
    m_target(0),
    m_target_deferred(0)
{
}


/**
OpticksAim::registerGeometry
------------------------------

Canonically invoked by OpticksHub::loadGeometry OR OpticksHub::adoptGeometry

**/

void OpticksAim::registerGeometry(GGeo* ggeo)
{
    assert( ggeo ); 
    m_ggeo = ggeo ; 

    //glm::vec4 ce0 = m_ggeo ? m_ggeo->getCE(0) : glm::vec4(0.f,0.f,0.f,1.f) ;

    int domaintarget = m_ok->getDomainTarget();    // --domaintarget 
    glm::vec4 center_extent = m_ggeo->getCE(domaintarget); 


    LOG(LEVEL)
          << " setting SpaceDomain : " 
          << " --domaintarget " << domaintarget
          << " center_extent " << gformat(center_extent) 
          ; 
    
    m_ok->setSpaceDomain( center_extent );
}

glm::vec4 OpticksAim::getCenterExtent() const 
{
    assert(0); 
    if(!m_ggeo) LOG(fatal) << " m_ggeo NULL " ; 
    glm::vec4 ce = m_ggeo ? m_ggeo->getCE(0) : glm::vec4(0.f,0.f,0.f,1.f) ;
    return ce ; 
}

void OpticksAim::dumpTarget(const char* msg) const 
{
    float extent_cut_mm = 5000.f ; 
    m_ggeo->dumpVolumes(msg, extent_cut_mm, m_target ); 
}


unsigned OpticksAim::getTargetDeferred() const 
{
    return m_target_deferred ;
}
unsigned OpticksAim::getTarget() const 
{
    return m_target ;
}


/**
OpticksAim::setupCompositionTargetting
-----------------------------------------

Relayed via OpticksHub::setupCompositionTargetting from eg OpticksViz::uploadGeometry or OpTracer::render

Decides on the target volume node index to configure for rendering. 
Priority order of inputs to control the target volume:

1. command line "--target 3155" option, with target defaulting to the value of OPTICKS_TARGET envvar or fallback 0  
2. deferred target if a request was made prior to geometry being loaded (is this still needed?)
3. GDMLAux metadata that annotates a logical volume via GDML auxiliary elements with (key,value) ("label","target"),
   the node index of the first physical placed volume instance of that logical volume

**/

void OpticksAim::setupCompositionTargetting()
{
    bool autocam = true ; 
    unsigned deferred_target = getTargetDeferred();   // default to 0   
    // ^^^^^^^^^^^^^ suspect no longer needed

    unsigned cmdline_target = m_ok->getTarget();      // sensitive to OPTICKS_TARGET envvar, fallback 0 

    const char* target_lvname = m_ok->getGDMLAuxTargetLVName() ; 
    int gdmlaux_target =  m_ggeo ? m_ggeo->getFirstNodeIndexForGDMLAuxTargetLVName() : -1 ;  // sensitive to GDML auxilary lvname metadata (label, target)  

    unsigned active_target = 0 ; 

    if( cmdline_target > 0 )
    {
        active_target = cmdline_target ; 
    } 
    else if( deferred_target > 0 )
    {
        active_target = deferred_target ; 
    } 
    else if( gdmlaux_target > 0 )
    {
        active_target = gdmlaux_target ;
    }

    LOG(error)
        << " deferred_target " << deferred_target
        << " cmdline_target " << cmdline_target
        << " target_lvname " << target_lvname
        << " gdmlaux_target " << gdmlaux_target  
        << " active_target " << active_target 
        ;   


    setTarget(active_target, autocam);
}

/**
OpticksAim::setTarget
----------------------

Sets the composition center_extent to that of the target 
volume identified by node index.

If this is invoked prior to registering geometry the 
target is retained in m_target_deferred.
   
Invoked by OpticksViz::uploadGeometry OpticksViz::init

Formerly of oglrap-/Scene

**/

void  OpticksAim::setTarget(unsigned target, bool aim)  
{
    assert(m_ggeo) ;  // surely always now available ?

    if(m_ggeo == NULL)
    {    
        LOG(LEVEL) << "target " << target << " (deferring as geometry not registered with OpticksAim) " ; 
        m_target_deferred = target ; 
    }    
    else
    {
        m_target = target ; 
        if(m_dbgaim) dumpTarget("OpticksAim::setTarget"); 

        glm::vec4 ce = m_ggeo->getCE(target);
        LOG(LEVEL)
            << " using CenterExtent from m_ggeo "
            << " target " << target 
            << " aim " << aim
            << " ce " << gformat(ce) 
            << " for details : --dbgaim " 
            ;    

        m_composition->setCenterExtent(ce, aim); 
    }
}


/**
OpticksAim::target
------------------

Sets composition center_extent depending on the target, 
presence of commandline option --geocenter and event gensteps.
This controls the view position and orientation used by rendering.

**/

void OpticksAim::target()
{
    int target_ = getTarget() ;
    bool geocenter  = m_ok->hasOpt("geocenter");  // --geocenter
    bool autocam = true ; 

    OpticksEvent* evt = m_hub->getEvent();

    if(target_ != 0)
    {
        LOG(LEVEL) << "SKIP as geometry target already set  " << target_ ; 
    }
    else if(geocenter )
    {
        //glm::vec4 ce0 = getCenterExtent();
        glm::vec4 ce0 = m_ggeo->getCE(0);
        m_composition->setCenterExtent( ce0 , autocam );
        LOG(LEVEL) << "[--geocenter] ce0 " << gformat(ce0) ; 
    }
    else if(evt && evt->hasGenstepData())
    {
        glm::vec4 gsce = evt->getGenstepCenterExtent();  // need to setGenStepData before this will work 
        m_composition->setCenterExtent( gsce , autocam );
        LOG(LEVEL) 
            << " evt " << evt->brief()
            << " gsce " << gformat(gsce) 
            ; 
    }
}


