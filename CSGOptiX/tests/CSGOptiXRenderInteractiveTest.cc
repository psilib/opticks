/**
CSGOptiXRenderInteractiveTest.cc : Interactive raytrace rendering of analytic geometry 
========================================================================================

Analytic CSGOptiX rendering with SGLM/SGLFW interactive control/visualization. 
Usage with::

   ~/o/CSGOptiX/cxr_min.sh 

Note this provides only CSGOptiX ray trace rendering, there is no rasterized toggle 
with "C" key to switch between CUDA and OpenGL rendering. 
For that functionality (but with triangulated geometry only) use:

   ~/o/sysrap/tests/SGLFW_SOPTIX_Scene_test.sh 
   ~/o/sysrap/tests/SGLFW_SOPTIX_Scene_test.cc 

TODO: 

* navigation frames currently managed in SScene(tri) but they 
  are equally relevant to ana+tri. 

  * Relocate the navigation frames, where ? Consider the workflow. 

* frame hopping like ~/o/sysrap/tests/SGLFW_SOPTIX_Scene_test.cc 

* interactive vizmask control to hide/show geometry 

* WIP: enable mix and match ana/tri geometry, by incorporation
  of SScene and SOPTIX 

* imgui reincarnation 

* WIP: moved some common CSGOptiX stuff down to sysrap header-only such as jpg/png writing and image annotation 

**/

#include "ssys.h"
#include "stree.h"
#include "OPTICKS_LOG.hh"
#include "SEventConfig.hh"
#include "CSGFoundry.h"
#include "CSGOptiX.h"
#include "SGLFW.h"
#include "SGLFW_CUDA.h"

#include "SScene.h"


int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 

    SEventConfig::SetRGModeRender(); 
    CSGFoundry* fd = CSGFoundry::Load(); 

    SScene* scene = fd->getScene(); 

    if(scene->is_empty())
    {
        LOG(fatal) << "CSGFoundry::Load GIVES EMPTY SCENE : TRANSITIONAL KLUDGE : TRY TO LOAD FROM SCENE_FOLD " ; 
        scene = SScene::Load("$SCENE_FOLD");   
        fd->setOverrideScene(scene); 
    }
    else
    {
        LOG(info) << "standard CSGFoundry::Load has scene : no need to kludge OverrideScene " ; 
    }


    static const char* _FRAME_HOP = "CSGOptiXRenderInteractiveTest__FRAME_HOP" ;  
    static const char* _SGLM_DESC = "CSGOptiXRenderInteractiveTest__SGLM_DESC" ;  
    bool FRAME_HOP = ssys::getenvbool(_FRAME_HOP); 
    bool SGLM_DESC = ssys::getenvbool(_SGLM_DESC); 


    CSGOptiX* cx = CSGOptiX::Create(fd) ;

    SGLM& gm = *(cx->sglm) ; 
    SGLFW gl(gm); 
    SGLFW_CUDA interop(gm); 

    stree* st = fd->getTree(); 
    assert(st);
 
    const char* MOI = ssys::getenvvar("MOI", nullptr);
    assert(MOI); 

    sfr mfr = st->get_frame(MOI); 
    mfr.set_idx(-2); 

    std::cout << "before loop  gl.get_wanted_frame_idx " <<  gl.get_wanted_frame_idx() << "\n" ; 
 
    while(gl.renderloop_proceed())
    {   
        gl.renderloop_head();


        // where to encapsulate this ? needs gl,gm,scene,mfr ?
        if(FRAME_HOP)
        {
            int wanted_frame_idx = gl.get_wanted_frame_idx() ; // -2 until press number key 0-9, back to -2 when press M  
            if(!gm.has_frame_idx(wanted_frame_idx) )
            {
                std::cout << _FRAME_HOP << " wanted_frame_idx: " << wanted_frame_idx << "\n"; 
                if( wanted_frame_idx == -2 )
                { 
                    gm.set_frame(mfr);  
                    if(SGLM_DESC) std::cout 
                         << _SGLM_DESC << "\n"  
                         << gm.desc() 
                         ; 
                }
                else if( wanted_frame_idx >= 0 )
                { 
                    assert(scene); 
                    sfr wfr = scene->getFrame(wanted_frame_idx) ; 
                    gm.set_frame(wfr);   
                }
            }
        }


        uchar4* d_pixels = interop.output_buffer->map() ; 

        cx->setExternalDevicePixels(d_pixels); 
        cx->render_launch(); 

        int wanted_snap = gl.get_wanted_snap();
        if( wanted_snap == 1 || wanted_snap == 2 )
        {
            std::cout << " gl.get_wanted_snap calling cx->render_snap \n" ; 
            switch(wanted_snap)
            {
                case 1: cx->render_save()          ; break ; 
                case 2: cx->render_save_inverted() ; break ; 
            }   
            gl.set_wanted_snap(0); 
        }

        interop.output_buffer->unmap() ; 
        interop.displayOutputBuffer(gl.window);

        gl.renderloop_tail();
    }   
    return 0 ; 
}

