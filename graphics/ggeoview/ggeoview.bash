# === func-gen- : graphics/ggeoview/ggeoview fgp graphics/ggeoview/ggeoview.bash fgn ggeoview fgh graphics/ggeoview
ggeoview-src(){      echo graphics/ggeoview/ggeoview.bash ; }
ggeoview-source(){   echo ${BASH_SOURCE:-$(env-home)/$(ggeoview-src)} ; }
ggeoview-vi(){       vi $(ggeoview-source) ; }
ggeoview-usage(){ cat << EOU

GGeoView
==========

Start from glfwtest- and add in OptiX functionality from optixrap-

* NB raytrace- is another user of optixwrap- 


Old way of doing interpolated photon position animation
-----------------------------------------------------------

* splayed out by maxsteps VBO

* recorded each photon step into its slot 

* pre-render CUDA time-presenter to find before and after 
  positions and interpolate between them writing into specific top slot 
  of the splay.


Problems:

* limited numbers of particles can be animated (perhaps 1000 or so)
  as approach multiplies storage by the max number of steps are kept

* most of the storage is empty, for photons without that many steps 

Advantages:

* splaying out allows CUDA to operate fully concurrently 
  with no atomics complexities, as every photon step has its place 
  in the structure 

* OpenGL can address and draw the VBO using fixed offsets/strides
  pointing at the interpolated slot, geometry shaders can be used to 
  amplify a point and momentum direction into a line


Improvement
-------------

Adopt separate minimal VBO for animation 

* single vec4 (position, time) ? 
* no need for direction as see that from the interpolation, 
* polz, wavelength, ... keep these in separate full-VBO for detailed debug 
  of small numbers of stepped photons 


Does modern OpenGL have any features that allow a better way
--------------------------------------------------------------

* http://gamedev.stackexchange.com/questions/20983/how-is-animation-handled-in-non-immediate-opengl

  * vertex blend-based animation
  * vertex blending
  * use glVertexAttribPointer to pick keyframes, 
  * shader gets two "position" attributes, 
    one for the keyframe in front of the current 
    time and one for the keyframe after and a uniform that specifies 
    how much of a blend to do between them. 


hmm not so easy 
    for photon simulation as they all are on their own timeline :
    so its not like traditional animation keyframes
 


glDrawArraysIndirect introduced in 4.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* https://www.opengl.org/wiki/History_of_OpenGL#OpenGL_4.1_.282010.29
* https://www.opengl.org/wiki/Vertex_Rendering#Indirect_rendering
* http://stackoverflow.com/questions/5047286/opengl-4-0-gpu-draw-feature
* https://www.opengl.org/registry/specs/ARB/draw_indirect.txt

Indirect rendering is the process of issuing a drawing command to OpenGL,
except that most of the parameters to that command come from GPU storage
provided by a Buffer Object.
The idea is to avoid the GPU->CPU->GPU round-trip; the GPU decides what range
of vertices to render with. All the CPU does is decide when to issue the
drawing command, as well as which Primitive is used with that command.

The indirect rendering functions take their data from the buffer currently
bound to the GL_DRAW_INDIRECT_BUFFER binding. Thus, any of these
functions will fail if no buffer is bound to that binding.

So can tee up a buffer of commands GPU side, following layout::

    void glDrawArraysIndirect(GLenum mode, const void *indirect);

    typedef  struct {
       GLuint  count;
       GLuint  instanceCount;
       GLuint  first;
       GLuint  baseInstance;   // MUST BE 0 IN 4.1
    } DrawArraysIndirectCommand;

Where each cmd is equivalent to::

    glDrawArraysInstancedBaseInstance(mode, cmd->first, cmd->count, cmd->instanceCount, cmd->baseInstance);

Similarly for indirect indexed drawing::

    glDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect);

    typedef  struct {
        GLuint  count;
        GLuint  instanceCount;
        GLuint  firstIndex;
        GLuint  baseVertex;
        GLuint  baseInstance;
    } DrawElementsIndirectCommand;

With each cmd equivalent to:: 

    glDrawElementsInstancedBaseVertexBaseInstance(mode, cmd->count, type,
      cmd->firstIndex * size-of-type, cmd->instanceCount, cmd->baseVertex, cmd->baseInstance);

* https://www.opengl.org/sdk/docs/man/html/glDrawElementsInstancedBaseVertex.xhtml


EOU
}


ggeoview-sdir(){ echo $(env-home)/graphics/ggeoview ; }
ggeoview-idir(){ echo $(local-base)/env/graphics/ggeoview ; }
ggeoview-bdir(){ echo $(ggeoview-idir).build ; }

ggeoview-ptx-dir(){ echo $(ggeoview-bdir)/lib/ptx ; }
ggeoview-ptx-ls(){  ls -l $(ggeoview-ptx-dir) ; }

ggeoview-scd(){  cd $(ggeoview-sdir); }
ggeoview-cd(){  cd $(ggeoview-sdir); }

ggeoview-icd(){  cd $(ggeoview-idir); }
ggeoview-bcd(){  cd $(ggeoview-bdir); }
ggeoview-name(){ echo GGeoView ; }

ggeoview-wipe(){
   local bdir=$(ggeoview-bdir)
   rm -rf $bdir
}
ggeoview-env(){     
    elocal- 
    optix-
    optix-export
}

ggeoview-cmake(){
   local iwd=$PWD

   local bdir=$(ggeoview-bdir)
   mkdir -p $bdir
  
   ggeoview-bcd 
   cmake \
       -DCMAKE_BUILD_TYPE=Debug \
       -DCMAKE_INSTALL_PREFIX=$(ggeoview-idir) \
       -DOptiX_INSTALL_DIR=$(optix-install-dir) \
       -DCUDA_NVCC_FLAGS="$(optix-cuda-nvcc-flags)" \
       $(ggeoview-sdir)

   cd $iwd
}

ggeoview-make(){
   local iwd=$PWD

   ggeoview-bcd 
   make $*

   cd $iwd
}

ggeoview-install(){
   ggeoview-make install
}

ggeoview-bin(){ echo $(ggeoview-idir)/bin/$(ggeoview-name) ; }
ggeoview-export()
{
   export-
   export-export

   export GGEOVIEW_GEOKEY="DAE_NAME_DYB"
   export GGEOVIEW_QUERY="range:5000:8000"
   #export GGEOVIEW_QUERY="index:5000"
   export GGEOVIEW_CTRL=""
   export SHADER_DIR=$(ggeoview-sdir)/gl

   export RAYTRACE_PTX_DIR=$(ggeoview-ptx-dir) 
} 
ggeoview-run(){ 
   local bin=$(ggeoview-bin)
   ggeoview-export
   $bin $*
}

ggeoview-lldb()
{
   local bin=$(ggeoview-bin)
   ggeoview-export
   lldb $bin $*
}

ggeoview--()
{
    ggeoview-wipe
    ggeoview-cmake
    ggeoview-make
    ggeoview-install
}



