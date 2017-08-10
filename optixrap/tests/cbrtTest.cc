#include "OptiXMinimalTest.hh"


int main( int argc, char** argv ) 
{
    optix::Context context = optix::Context::create();

    assert( argc == 3 );
    const char* ptxpath = argv[1] ;
    const char* raygen = argv[2] ;

    std::cout << " ptxpath " << ptxpath
              << " raygen " << raygen 
              << std::endl ; 

    OptiXMinimalTest* test = new OptiXMinimalTest(context, ptxpath, raygen ) ;

    std::cout << test->description() << std::endl ; 

    //unsigned width = 512 ; 
    //unsigned height = 512 ; 

    unsigned width = 1 ; 
    unsigned height = 1 ; 


    optix::Buffer buffer = context->createBuffer( RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, width*height );
    context["output_buffer"]->set(buffer);

    context->validate();
    context->compile();
    context->launch(0, width, height);

    return 0;
}
