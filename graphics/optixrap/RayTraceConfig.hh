#ifndef RAYTRACECONFIG_H
#define RAYTRACECONFIG_H

// compiled in constants are not very convenient
//#include "RayTraceConfigInc.h"

#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>

#include <string>
#include <map>

class RayTraceConfig {

public:
   // singleton 
   static RayTraceConfig* g_instance ; 
   static RayTraceConfig* getInstance();
   static RayTraceConfig* makeInstance(optix::Context context, const char* target);

public:
   // using the compiled in constants generated by cmake in RayTraceConfigInc.h 
   static const char* SrcDir();
   static const char* PtxDir();
   static const char* RngDir();

   static void Print(const char* msg="RayTraceConfig::Print");
   static const char* const ptxpath( const std::string& target, const std::string& base );

   static optix::float3 make_contrast_color(int tag);
   static unsigned int getMultiplicity(RTformat format);

public:
    RayTraceConfig(optix::Context context, const char* target);
    virtual ~RayTraceConfig();

    const char* const ptxpath( const char* filename );
    optix::Program createProgram(const char* filename, const char* fname );

    void setRayGenerationProgram( unsigned int index , const char* filename, const char* fname );
    void setExceptionProgram( unsigned int index , const char* filename, const char* fname );
    void setMissProgram( unsigned int index , const char* filename, const char* fname );

private:
    optix::Context m_context ;

    char* m_target ;

    std::map<std::string,optix::Program> m_programs;


};

#endif
