# === func-gen- : cuda/optix/optix fgp cuda/optix/optix.bash fgn optix fgh cuda/optix
optix-src(){      echo cuda/optix/optix.bash ; }
optix-source(){   echo ${BASH_SOURCE:-$(env-home)/$(optix-src)} ; }
optix-vi(){       vi $(optix-source) ; }
optix-env(){      elocal- ; }
optix-usage(){ cat << EOU

NVIDIA OptiX Ray Trace Toolkit
================================== 

Resources
----------

* https://devtalk.nvidia.com/default/board/90/optix/

* http://on-demand-gtc.gputechconf.com/gtcnew/on-demand-gtc.php?searchByKeyword=Optix&searchItems=&sessionTopic=&sessionEvent=&sessionYear=&sessionFormat=&submit=&select=+

  Many presentations (videos and pdfs) on OptiX


Release Notes OptiX Version 3.7 beta 3 (January 2015)
--------------------------------------------------------

The CUDA R343 or later driver is required. The latest available WHQL drivers
are highly recommended. For the Mac, the driver extension module supplied with
CUDA 5.0 or later will need to be installed. Note that the Linux and Mac
drivers can only be obtained from the CUDA 6.5 download page at the moment.

SLI is not required for OptiX to use multiple GPUs, and it interferes when
OptiX uses either D3D or OpenGL interop. Disabling SLI will not degrade OptiX
performance and will provide a more stable environment for OptiX applications
to run. SLI is termed “Multi-GPU mode” in recent NVIDIA Control Panels, with
the correct option being “Disable multi-GPU mode” to ensure OptiX is not
encumbered by graphics overhead.


hgpu01 install
--------------

Manual CUDA::

    -bash-4.1$ cat /etc/redhat-release 
    Scientific Linux release 6.5 (Carbon)

    -bash-4.1$ rpm -qf /usr/local/cuda-5.5
    file /usr/local/cuda-5.5 is not owned by any package

Hmm getting "OptiX Error: Invalid context", with 370b3 
I was getting this on laptop until I updated the driver.

::

    scp /Users/blyth/Downloads/NVIDIA-OptiX-SDK-3.5.1-linux64.run L6:/dyb/dybd07/user/blyth/hgpu01.ihep.ac.cn/
    scp /Users/blyth/Downloads/NVIDIA-OptiX-SDK-3.6.3-linux64.run L6:/dyb/dybd07/user/blyth/hgpu01.ihep.ac.cn/
    scp /Users/blyth/Downloads/NVIDIA-OptiX-SDK-3.7.0-linux64.sh L6:/dyb/dybd07/user/blyth/hgpu01.ihep.ac.cn/

    -bash-4.1$ chmod ugo+x NVIDIA-OptiX-SDK-3.5.1-linux64.run 
    -bash-4.1$ chmod ugo+x NVIDIA-OptiX-SDK-3.6.3-linux64.run 
    -bash-4.1$ chmod ugo+x NVIDIA-OptiX-SDK-3.7.0-linux64.sh 

    -bash-4.1$ ./NVIDIA-OptiX-SDK-3.5.1-linux64.run --prefix=. --include-subdir
    -bash-4.1$ ./NVIDIA-OptiX-SDK-3.6.3-linux64.run --prefix=. --include-subdir
    -bash-4.1$ ./NVIDIA-OptiX-SDK-3.7.0-linux64.sh --prefix=. --include-subdir

    -bash-4.1$ ln -sfnv NVIDIA-OptiX-SDK-3.5.1-PRO-linux64 OptiX
    -bash-4.1$ ln -sfnv NVIDIA-OptiX-SDK-3.6.3-linux64 OptiX
    -bash-4.1$ ln -sfnv NVIDIA-OptiX-SDK-3.7.0-linux64 OptiX

    -bash-4.1$ pwd
    /dyb/dybd07/user/blyth/hgpu01.ihep.ac.cn/OptiX/SDK-precompiled-samples

    -bash-4.1$ LD_LIBRARY_PATH=. ./sample3
    OptiX 3.7.0
    Number of Devices = 2

    Device 0: Tesla K20m
      Compute Support: 3 5
      Total Memory: 5032706048 bytes
      Clock Rate: 705500 kilohertz
      Max. Threads per Block: 1024
      SM Count: 13
      Execution Timeout Enabled: 0
      Max. HW Texture Count: 128
      TCC driver enabled: 0
      CUDA Device Ordinal: 0

    Device 1: Tesla K20m
      Compute Support: 3 5
      Total Memory: 5032706048 bytes
      Clock Rate: 705500 kilohertz
      Max. Threads per Block: 1024
      SM Count: 13
      Execution Timeout Enabled: 0
      Max. HW Texture Count: 128
      TCC driver enabled: 0
      CUDA Device Ordinal: 1

    Constructing a context...
    OptiX Error: Invalid context
    (/root/sw/wsapps/raytracing/rtsdk/rel3.7/samples/sample3/sample3.c:96)

    -bash-4.1$ LD_LIBRARY_PATH=. ./sample3
    OptiX 3.5.1
    Number of Devices = 2

    Device 0: Tesla K20m
      Compute Support: 3 5
      Total Memory: 5032706048 bytes
      Clock Rate: 705500 kilohertz
      Max. Threads per Block: 1024
      SM Count: 13
      Execution Timeout Enabled: 0
      Max. HW Texture Count: 128
      TCC driver enabled: 0
      CUDA Device Ordinal: 0

    Device 1: Tesla K20m
      Compute Support: 3 5
      Total Memory: 5032706048 bytes
      Clock Rate: 705500 kilohertz
      Max. Threads per Block: 1024
      SM Count: 13
      Execution Timeout Enabled: 0
      Max. HW Texture Count: 128
      TCC driver enabled: 0
      CUDA Device Ordinal: 1

    Constructing a context...
      Created with 2 device(s)
      Supports 2147483647 simultaneous textures
      Free memory:
        Device 0: 4952547328 bytes
        Device 1: 4952547328 bytes


    -bash-4.1$ LD_LIBRARY_PATH=. ./sample3
    OptiX 3.6.3
    Number of Devices = 2

    Device 0: Tesla K20m
      Compute Support: 3 5
      Total Memory: 5032706048 bytes
      Clock Rate: 705500 kilohertz
      Max. Threads per Block: 1024
      SM Count: 13
      Execution Timeout Enabled: 0
      Max. HW Texture Count: 128
      TCC driver enabled: 0
      CUDA Device Ordinal: 0

    Device 1: Tesla K20m
      Compute Support: 3 5
      Total Memory: 5032706048 bytes
      Clock Rate: 705500 kilohertz
      Max. Threads per Block: 1024
      SM Count: 13
      Execution Timeout Enabled: 0
      Max. HW Texture Count: 128
      TCC driver enabled: 0
      CUDA Device Ordinal: 1

    Constructing a context...
      Created with 2 device(s)
      Supports 2147483647 simultaneous textures
      Free memory:
        Device 0: 4952547328 bytes
        Device 1: 4952547328 bytes






Transparency/blending
-----------------------

* https://developer.nvidia.com/content/transparency-or-translucency-rendering

* http://casual-effects.blogspot.tw/2014/03/weighted-blended-order-independent.html


OptiX and atomics
-------------------

* https://devtalk.nvidia.com/default/topic/522795/optix/atomic-buffer-operations/

  see zoneplate sample


  One thing to keep in mind is that atomic operations will not work in multi-GPU
  situations unless you specify RT_BUFFER_GPU_LOCAL. In that case the data stays
  resident on the device and can only be read by the device that wrote it.


OptiX and curand ?
-------------------

* :google:`optix curand`

  * https://devtalk.nvidia.com/default/topic/759883/random-number-streams/?offset=1

Caveats
----------

* https://devtalk.nvidia.com/default/topic/751906/?comment=4240594
  
  * rtPrintf not printf


OptiX Usage Examples
---------------------

* https://code.google.com/p/hybrid-rendering-thesis/source/browse/trunk/src/Raytracer/OptixRender.cpp?r=44

* https://github.com/keithroe/Legion/blob/master/src/Legion/Renderer/OptiXScene.cpp

* https://github.com/pspkzar/OptiXRenderer/blob/master/src/OptixRenderer.cpp

* https://github.com/nvpro-samples/gl_optix_composite

* http://graphicsrunner.blogspot.tw/2011/03/instant-radiosity-using-optix-and.html



Version Switching
------------------

Use symbolic link for version switching::

    delta:Developer blyth$ ll
    total 8
    drwxr-xr-x   7 root  admin   238 Aug  7  2013 OptiX_301
    drwxr-xr-x   3 root  wheel   102 Jan 15  2014 NVIDIA
    drwxr-xr-x   7 root  admin   238 Dec 18 07:08 OptiX_370b2
    drwxr-xr-x  33 root  wheel  1190 Jan 15 08:46 ..
    lrwxr-xr-x   1 root  wheel     9 Jan 22 11:27 OptiX -> OptiX_301
    drwxr-xr-x   6 root  wheel   204 Jan 22 11:27 .


Building samples including sutil library 
-------------------------------------------

::

    delta:OptiX blyth$ optix-name
    OptiX_370b2
    delta:OptiX blyth$ optix-samples-get-all   # copy samples to writable dir
    delta:OptiX blyth$ optix-samples-cmake
    ...
    -- Found CUDA: /usr/local/cuda (Required is at least version "2.3") 
    -- Found OpenGL: /System/Library/Frameworks/OpenGL.framework  
    -- Found GLUT: -framework GLUT  
    Cannot find Cg, hybridShadows will not be built
    Cannot find Cg.h, hybridShadows will not be built
    Disabling hybridShadows, which requires glut and opengl and Cg.
    Cannot find Cg, isgReflections will not be built
    Cannot find Cg.h, isgReflections will not be built
    Disabling isgReflections, which requires glut and opengl and Cg.
    Cannot find Cg, isgShadows will not be built
    Cannot find Cg.h, isgShadows will not be built
    Disabling isgShadows, which requires glut and opengl and Cg.
    -- Configuring done
    -- Generating done
    -- Build files have been written to: /usr/local/env/cuda/OptiX_370b2_sdk_install

    delta:OptiX blyth$ optix-samples-make





Path to SAMPLES_PTX_DIR gets compiled in
-------------------------------------------

::

    delta:SDK blyth$ find . -name '*.*' -exec grep -H SAMPLES_PTX_DIR {} \;
    ./CMakeLists.txt:set(SAMPLES_PTX_DIR "${CMAKE_BINARY_DIR}/lib/ptx" CACHE PATH "Path to where the samples look for the PTX code.")
    ./CMakeLists.txt:set(CUDA_GENERATED_OUTPUT_DIR ${SAMPLES_PTX_DIR})
    ./CMakeLists.txt:  string(REPLACE "/" "\\\\" SAMPLES_PTX_DIR ${SAMPLES_PTX_DIR})
    ./sampleConfig.h.in:#define SAMPLES_PTX_DIR "@SAMPLES_PTX_DIR@"
    ./sutil/sutil.c:  dir = getenv( "OPTIX_SAMPLES_PTX_DIR" );
    ./sutil/sutil.c:  if( dirExists(SAMPLES_PTX_DIR) )
    ./sutil/sutil.c:    return SAMPLES_PTX_DIR;



OptiX-3.7.0-beta2
-------------------

* need to register with NVIDIA OptiX developer program to gain access 

Package installs into same place as 301::

    delta:Contents blyth$ pwd
    /Volumes/NVIDIA-OptiX-SDK-3.7.0-mac64/NVIDIA-OptiX-SDK-3.7.0-mac64.pkg/Contents
    delta:Contents blyth$ lsbom Archive.bom | head -5
    .   40755   501/0
    ./Developer 40755   501/0
    ./Developer/OptiX   40755   0/80
    ./Developer/OptiX/SDK   40755   0/80
    ./Developer/OptiX/SDK/CMake 40755   0/80

So move that aside::

    delta:Developer blyth$ sudo mv OptiX OptiX_301


* all precompiled samples failing 

::

    terminating with uncaught exception of type optix::Exception: Invalid context

    8   libsutil.dylib                  0x000000010f8b71d6 optix::Handle<optix::ContextObj>::create() + 150
    9   libsutil.dylib                  0x000000010f8b5b1b SampleScene::SampleScene() + 59
    10  libsutil.dylib                  0x000000010f8a6a52 MeshScene::MeshScene(bool, bool, bool) + 34
    11                                  0x000000010f870885 MeshViewer::MeshViewer() + 21


    delta:SDK-precompiled-samples blyth$ open ocean.app
    LSOpenURLsWithRole() failed with error -10810 for the file /Developer/OptiX/SDK-precompiled-samples/ocean.app.
    delta:SDK-precompiled-samples blyth$ 


    8   libsutil.dylib                  0x000000010e1141d6 optix::Handle<optix::ContextObj>::create() + 150
    9   libsutil.dylib                  0x000000010e112b1b SampleScene::SampleScene() + 59
    10                                  0x000000010e0d793c WhirligigScene::WhirligigScene(GLUTDisplay::contDraw_E) + 28



::

    delta:SDK-precompiled-samples blyth$ ./sample3
    OptiX 3.7.0
    Number of Devices = 1

    Device 0: GeForce GT 750M
      Compute Support: 3 0
      Total Memory: 2147024896 bytes
      Clock Rate: 925500 kilohertz
      Max. Threads per Block: 1024
      SM Count: 2
      Execution Timeout Enabled: 1
      Max. HW Texture Count: 128
      TCC driver enabled: 0
      CUDA Device Ordinal: 0

    Constructing a context...
    OptiX Error: Invalid context
    (/Volumes/DATA/teamcity/agent/work/ad29186266c461fa/sw/wsapps/raytracing/rtsdk/rel3.7/samples/sample3/sample3.c:96)
    delta:SDK-precompiled-samples blyth$ 

::

     95   printf("Constructing a context...\n");
     96   RT_CHECK_ERROR(rtContextCreate(&context));
     97 



This is with

* CUDA Driver Version: 5.5.47
* GPU Driver Version: 8.26.26 310.40.45f01



OptiX 301 Install issues 
--------------------------

::

    delta:~ blyth$ optix-cmake
    ...
    Specified C compiler /usr/bin/cc is not recognized (gcc, icc).  Using CMake defaults.
    Specified CXX compiler /usr/bin/c++ is not recognized (g++, icpc).  Using CMake defaults.
    CMake Warning at CMake/ConfigCompilerFlags.cmake:195 (message):
      Unknown Compiler.  Disabling SSE 4.1 support
    Call Stack (most recent call first):
      CMakeLists.txt:116 (include)


    -- Found CUDA: /usr/local/cuda (Required is at least version "2.3") 
    -- Found OpenGL: /System/Library/Frameworks/OpenGL.framework  
    -- Found GLUT: -framework GLUT  
    Cannot find Cg, hybridShadows will not be built
    Cannot find Cg.h, hybridShadows will not be built
    Disabling hybridShadows, which requires glut and opengl and Cg.
    Cannot find Cg, isgShadows will not be built
    Cannot find Cg.h, isgShadows will not be built
    Disabling isgShadows, which requires glut and opengl and Cg.
    Cannot find Cg, isgReflections will not be built
    Cannot find Cg.h, isgReflections will not be built
    Disabling isgReflections, which requires glut and opengl and Cg.
    -- Configuring done
    -- Generating done
    -- Build files have been written to: /usr/local/env/cuda/optix301
    delta:optix301 blyth$ 


List the samples::

    delta:optix301 blyth$ optix-make help

All giving error::

    delta:optix301 blyth$ optix-make sample6
    [  7%] Building NVCC ptx file lib/ptx/cuda_compile_ptx_generated_triangle_mesh_small.cu.ptx
    clang: error: unsupported option '-dumpspecs'
    clang: error: no input files
    CMake Error at cuda_compile_ptx_generated_triangle_mesh_small.cu.ptx.cmake:200 (message):
      Error generating
      /usr/local/env/cuda/optix301/lib/ptx/cuda_compile_ptx_generated_triangle_mesh_small.cu.ptx


    make[3]: *** [lib/ptx/cuda_compile_ptx_generated_triangle_mesh_small.cu.ptx] Error 1
    make[2]: *** [sutil/CMakeFiles/sutil.dir/all] Error 2
    make[1]: *** [sample6/CMakeFiles/sample6.dir/rule] Error 2
    make: *** [sample6] Error 2
    delta:optix301 blyth$ 

Seems that nvcc is running clang internally with non existing option::

    delta:optix301 blyth$ /usr/local/cuda/bin/nvcc -M -D__CUDACC__ /Developer/OptiX/SDK/cuda/triangle_mesh_small.cu -o /usr/local/env/cuda/optix301/sutil/CMakeFiles/cuda_compile_ptx.dir/__/cuda/cuda_compile_ptx_generated_triangle_mesh_small.cu.ptx.NVCC-depend -ccbin /usr/bin/cc -m64 -DGLUT_FOUND -DGLUT_NO_LIB_PRAGMA --use_fast_math -U__BLOCKS__ -DNVCC -I/usr/local/cuda/include -I/Developer/OptiX/include -I/Developer/OptiX/SDK/sutil -I/Developer/OptiX/include/optixu -I/usr/local/env/cuda/optix301 -I/usr/local/cuda/include -I/System/Library/Frameworks/GLUT.framework/Headers -I/Developer/OptiX/SDK/sutil -I/Developer/OptiX/SDK/cuda
    clang: error: unsupported option '-dumpspecs'
    clang: error: no input files
    delta:optix301 blyth$ 


cmake debug
~~~~~~~~~~~~~

* added "--verbose"
* adding "-ccbin /usr/bin/clang" gets past the "--dumpspecs" failure, now get

    nvcc fatal   : redefinition of argument 'compiler-bindir'


* /Developer/OptiX/SDK/CMake/FindCUDA/run_nvcc.cmake::

    108 # Any -ccbin existing in CUDA_NVCC_FLAGS gets highest priority
    109 list( FIND CUDA_NVCC_FLAGS "-ccbin" ccbin_found0 )
    110 list( FIND CUDA_NVCC_FLAGS "--compiler-bindir" ccbin_found1 )
    111 if( ccbin_found0 LESS 0 AND ccbin_found1 LESS 0 )
    112   if (CUDA_HOST_COMPILER STREQUAL "$(VCInstallDir)bin" AND DEFINED CCBIN)
    113     set(CCBIN -ccbin "${CCBIN}")
    114   else()
    115     set(CCBIN -ccbin "${CUDA_HOST_COMPILER}")
    116   endif()
    117 endif()
     
    * http://public.kitware.com/Bug/view.php?id=13674


cmake fix
~~~~~~~~~~~~~~


Kludge the cmake::

    delta:FindCUDA blyth$ sudo cp run_nvcc.cmake run_nvcc.cmake.original
    delta:FindCUDA blyth$ pwd
    /Developer/OptiX/SDK/CMake/FindCUDA

Turns out not to be necessary, the cmake flag does the trick::

   cmake $(optix-dir) -DCUDA_NVCC_FLAGS="-ccbin /usr/bin/clang"
    

* :google:`cuda 5.5 clang`
* http://stackoverflow.com/questions/19351219/cuda-clang-and-os-x-mavericks
* http://stackoverflow.com/questions/12822205/nvidia-optix-geometrygroup


Check Optix Raytrace Speed on DYB geometry
--------------------------------------------

::

    In [3]: v=np.load(os.path.expandvars("$DAE_NAME_DYB_CHROMACACHE_MESH/vertices.npy"))

    In [4]: v
    Out[4]: 
    array([[ -16585.725, -802008.375,   -3600.   ],
           [ -16039.019, -801543.125,   -3600.   ],
           [ -15631.369, -800952.188,   -3600.   ],
           ..., 
           [ -14297.924, -801935.812,  -12110.   ],
           [ -14414.494, -801973.438,  -12026.   ],
           [ -14414.494, -801973.438,  -12110.   ]], dtype=float32)

    In [5]: v.shape
    Out[5]: (1216452, 3)

    In [6]: t = np.load(os.path.expandvars("$DAE_NAME_DYB_CHROMACACHE_MESH/triangles.npy"))
    In [7]: t.shape
    Out[7]: (2402432, 3)
    In [8]: t.max()
    Out[8]: 1216451
    In [9]: t.min()
    Out[9]: 0


Write geometry in obj format::

    In [11]: fp = file("/tmp/dyb.obj", "w")
    In [12]: np.savetxt(fp, v, fmt="v %.18e %.18e %.18e")
    In [13]: np.savetxt(fp, t, fmt="f %d %d %d")
    In [14]: fp.close()

Geometry appears mangled, as obj format does not handle Russian doll geometry, 
but the optix raytrace is interactive (unless some trickery being used, that is 
greatly faster than chroma raytrace). Fast enough to keep me interested::

    ./sample6 --cache --obj /tmp/dyb.obj --light-scale 5


How to load COLLADA into OptiX ?
-----------------------------------

* nvidia Scenix looks abandoned

* plumped for assimp following example of oppr- example, see assimp- assimptest- raytrace--

* oppr- converts ASSIMP imported mesh into OptiX geometry::

    delta:OppositeRenderer blyth$ find . -name '*.cpp' -exec grep -H getSceneRootGroup {} \;
    ./RenderEngine/renderer/OptixRenderer.cpp:        m_sceneRootGroup = scene.getSceneRootGroup(m_context);
    ./RenderEngine/scene/Cornell.cpp:optix::Group Cornell::getSceneRootGroup(optix::Context & context)
    ./RenderEngine/scene/Scene.cpp:optix::Group Scene::getSceneRootGroup( optix::Context & context )
    delta:OppositeRenderer blyth$ 


OptiX Tutorial
---------------

* http://docs.nvidia.com/gameworks/content/gameworkslibrary/optix/optix_quickstart.htm

Tutorials gt 5 asserting in rtContextCompile::

    delta:bin blyth$ ./tutorial -T 5 
    OptiX Error: Unknown error (Details: Function "RTresult _rtContextCompile(RTcontext_api*)" caught exception: Assertion failed: [1312612])

Binary search reveals the culprit to be the *sin(phi)*::

     74 rtTextureSampler<float4, 2> envmap;
     75 RT_PROGRAM void envmap_miss()
     76 {
     77   float theta = atan2f( ray.direction.x, ray.direction.z );
     78   float phi   = M_PIf * 0.5f -  acosf( ray.direction.y );
     79   float u     = (theta + M_PIf) * (0.5f * M_1_PIf);
     80   float v     = 0.5f * ( 1.0f + sin(phi) );
     81   // the "sin" above causing an assert with OptiX_301 CUDA 5.5 without --use_fast_math 
     82   prd_radiance.result = make_float3( tex2D(envmap, u, v) );
     83 } 

* https://devtalk.nvidia.com/default/topic/559505/apparently-an-unexplicable-error/

Resolved by adding *--use_fast_math* to the cmake commandline setting CUDA_NVCC_FLAGS::

   cmake -DOptiX_INSTALL_DIR=$(optix-install-dir) -DCUDA_NVCC_FLAGS="-ccbin /usr/bin/clang --use_fast_math " "$(optix-sdir)"

After a few crashes like the above observe GPU memory to be almost full
and attempts to run anything on the GPU fail with a system 
exception report. To free up some GPU memory sleep/revive the machine::

    delta:bin blyth$ cu
    timestamp                Fri Jan 23 10:44:11 2015
    tag                      default
    name                     GeForce GT 750M
    compute capability       (3, 0)
    memory total             2.1G
    memory used              2.1G
    memory free              51.4M
    delta:bin blyth$ 


f64 check
-----------

::

    delta:raytrace blyth$ grep f64 *.ptx
    MeshViewer_generated_TriangleMesh.cu.ptx:   .target sm_10, map_f64_to_f32
    MeshViewer_generated_TriangleMesh.cu.ptx:   ld.global.f32   %f64, [ray+28];
    MeshViewer_generated_TriangleMesh.cu.ptx:   set.lt.u32.f32  %r30, %f64, %f58;
    MeshViewer_generated_material1.cu.ptx:  .target sm_10, map_f64_to_f32
    RayTrace_generated_TriangleMesh.cu.ptx: .target sm_10, map_f64_to_f32
    RayTrace_generated_TriangleMesh.cu.ptx: ld.global.f32   %f64, [ray+28];
    RayTrace_generated_TriangleMesh.cu.ptx: set.lt.u32.f32  %r30, %f64, %f58;
    RayTrace_generated_material0.cu.ptx:    .target sm_10, map_f64_to_f32
    RayTrace_generated_material1.cu.ptx:    .target sm_10, map_f64_to_f32
    RayTrace_generated_tutorial0.cu.ptx:    .target sm_10, map_f64_to_f32
    RayTrace_generated_tutorial0.cu.ptx:    cvt.sat.f32.f32     %f64, %f63;
    RayTrace_generated_tutorial0.cu.ptx:    mul.f32     %f66, %f64, %f65;
    delta:raytrace blyth$ 


Following updating CUDA from 5.5 to 6.5 get
---------------------------------------------

While still using OptiX301::

    delta:sample3 blyth$ raytrace-v -n
    [ 19%] Built target AssimpGeometryTest
    Scanning dependencies of target MeshViewer
    [ 22%] Building CXX object CMakeFiles/MeshViewer.dir/MeshViewer.cpp.o
    Linking CXX executable MeshViewer
    [ 61%] Built target MeshViewer
    [100%] Built target RayTrace
    dyld: Library not loaded: @rpath/libcudart.dylib
      Referenced from: /Developer/OptiX/lib64/liboptix.1.dylib
      Reason: Incompatible library version: liboptix.1.dylib requires version 1.1.0 or later, but libcudart.5.5.dylib provides version 0.0.0
    Trace/BPT trap: 5
    delta:sample3 blyth$ 



But with the beta OptiX_370b2 the invalid context issue is gone::

delta:SDK-precompiled-samples blyth$ ./sample3
OptiX 3.7.0
Number of Devices = 1

Device 0: GeForce GT 750M
  Compute Support: 3 0
  Total Memory: 2147024896 bytes
  Clock Rate: 925500 kilohertz
  Max. Threads per Block: 1024
  SM Count: 2
  Execution Timeout Enabled: 1
  Max. HW Texture Count: 128
  TCC driver enabled: 0
  CUDA Device Ordinal: 0

Constructing a context...
  Created with 1 device(s)
  Supports 2147483647 simultaneous textures
  Free memory:
    Device 0: 1099292672 bytes





EOU
}




optix-export(){
   export OPTIX_SDK_DIR=$(optix-sdk-dir)
   export OPTIX_INSTALL_DIR=$(optix-install-dir)
   export OPTIX_SAMPLES_INSTALL_DIR=$(optix-samples-install-dir)
}

optix-fold(){ 
   case $NODE_TAG in 
      D)  echo /Developer ;;
      G1) echo $(local-base) ;;
   esac
}
optix-dir(){     echo $(optix-fold)/OptiX/SDK ; }
optix-sdk-dir-old(){ echo $(optix-fold)/OptiX_301/SDK ; }
optix-sdk-dir(){ echo $(optix-fold)/OptiX/SDK ; }
optix-install-dir(){ echo $(dirname $(optix-sdk-dir)) ; }
optix-idir(){ echo $(dirname $(optix-sdk-dir))/include ; }
optix-bdir(){ echo $(local-base)/env/cuda/$(optix-name) ; }
optix-sdir(){ echo $(env-home)/cuda/optix/$(optix-name) ; }

optix-samples-src-dir(){    echo $(local-base)/env/cuda/$(optix-name)_sdk ; }
optix-samples-install-dir(){ echo $(local-base)/env/cuda/$(optix-name)_sdk_install ; }
optix-samples-scd(){ cd $(optix-samples-src-dir) ; }
optix-samples-cd(){ cd $(optix-samples-install-dir) ; }

optix-cd(){  cd $(optix-dir); }
optix-bcd(){ cd $(optix-samples-install-dir); }
optix-scd(){ cd $(optix-sdir); }
optix-icd(){ cd $(optix-idir); }



optix-name(){   readlink $(optix-fold)/OptiX ; }
optix-jump(){    
   local iwd=$PWD
   local ver=${1:-301}
   cd $(optix-fold)
   sudo ln -sfnv OptiX_$ver OptiX 
   cd $iwd
}
optix-old(){   optix-jump 301 ; }
optix-beta(){  optix-jump 370b2 ; }


optix-samples-names(){ cat << EON
CMakeLists.txt
sampleConfig.h.in
cuda
CMake
sample1
sample2
sample3
sample4
sample5
sample5pp
sample6
sample7
sample8
simpleAnimation
sutil
tutorial
materials
transparency
EON

## remember that after adding a name here, need to uncomment in the CMakeLists.txt for it to get built
}



optix-samples-get-all(){

   local src=$(optix-sdk-dir)
   local dst=$(optix-samples-src-dir)
 
   mkdir -p $dst

   echo $FUNCNAME copy all samples to somewhere writable 
   cp -R $src/* $dst/
 
}


optix-samples-get-selected(){
   local sdir=$(optix-samples-src-dir)
   mkdir -p $sdir

   local src=$(optix-sdk-dir)
   local dst=$sdir
   local cmd
   local name
   optix-samples-names | while read name ; do 

      if [ -d "$src/$name" ]
      then 
          if [ ! -d "$dst/$name" ] ; then 
              cmd="cp -r $src/$name $dst/"
          else
              cmd="echo destination directory exists already $dst/$name"
          fi
      elif [ -f "$src/$name" ] 
      then 
          if [ ! -f "$dst/$name" ] ; then 
              cmd="cp $src/$name $dst/$name"
          else
              cmd="echo destination file exists already $dst/$name"
          fi
      else
          cmd="echo src $src/$name missing"
      fi 
      #echo $cmd
      eval $cmd
   done
}


optix-samples-cmake(){
    local iwd=$PWD
    local bdir=$(optix-samples-install-dir)
    #rm -rf $bdir   # starting clean 
    mkdir -p $bdir
    optix-bcd
    cmake -DOptiX_INSTALL_DIR=$(optix-install-dir) -DCUDA_NVCC_FLAGS="-ccbin /usr/bin/clang --use_fast_math " "$(optix-samples-src-dir)"
    cd $iwd
}

optix-samples-make(){
    local iwd=$PWD
    optix-bcd
    make $* 
    cd $iwd
}




optix-samples-run(){
    local name=${1:-materials}
    optix-samples-make $name
    local cmd="$(optix-bdir)/bin/$name"
    echo $cmd
    eval $cmd
}

optix-tutorial(){
    local tute=${1:-10}

    optix-samples-make tutorial

    local cmd="$(optix-bdir)/bin/tutorial -T $tute --texture-path $(optix-sdk-dir)/tutorial/data"
    echo $cmd
    eval $cmd
}


optix-tutorial-cd(){
   cd $(optix-sdk-dir)/tutorial
}
optix-tutorial-vi(){
   vi $(optix-sdk-dir)/tutorial/*
}




optix-verbose(){
  export VERBOSE=1 
}
optix-unverbose(){
  unset VERBOSE
}



optix-check(){
/usr/local/cuda/bin/nvcc -ccbin /usr/bin/clang --verbose -M -D__CUDACC__ /Developer/OptiX/SDK/cuda/triangle_mesh_small.cu -o /usr/local/env/cuda/optix301/sutil/CMakeFiles/cuda_compile_ptx.dir/__/cuda/cuda_compile_ptx_generated_triangle_mesh_small.cu.ptx.NVCC-depend -ccbin /usr/bin/cc -m64 -DGLUT_FOUND -DGLUT_NO_LIB_PRAGMA --use_fast_math -U__BLOCKS__ -DNVCC -I/usr/local/cuda/include -I/Developer/OptiX/include -I/Developer/OptiX/SDK/sutil -I/Developer/OptiX/include/optixu -I/usr/local/env/cuda/optix301 -I/usr/local/cuda/include -I/System/Library/Frameworks/GLUT.framework/Headers -I/Developer/OptiX/SDK/sutil -I/Developer/OptiX/SDK/cuda
}



optix-check-2(){

cd /usr/local/env/cuda/OptiX_301/tutorial && /usr/bin/c++   -DGLUT_FOUND -DGLUT_NO_LIB_PRAGMA -fPIC -O3 -DNDEBUG \
     -I/Developer/OptiX/include \
     -I/Users/blyth/env/cuda/optix/OptiX_301/sutil \
     -I/Developer/OptiX/include/optixu \
     -I/usr/local/env/cuda/OptiX_301 \
     -I/usr/local/cuda/include \
     -I/System/Library/Frameworks/GLUT.framework/Headers \
       -o /dev/null \
       -c /Users/blyth/env/cuda/optix/OptiX_301/tutorial/tutorial.cpp

}



optix-diff(){
   local name=${1:-sutil/MeshScene.h}
   local cmd="diff $(optix-sdk-dir-old)/$name $(optix-sdk-dir)/$name"
   echo $cmd
   eval $cmd
}

optix-rdiff(){
   local rel="sutil"
   local cmd="diff -r --brief $(optix-sdk-dir-old)/$rel $(optix-sdk-dir)/$rel"
   echo $cmd
   eval $cmd
}





