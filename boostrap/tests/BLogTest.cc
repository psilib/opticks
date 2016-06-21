#include "BLog.hh"

int main(int argc, char** argv)
{
    BLog bl(argc, argv);

    LOG(trace) << argv[0] ;
    LOG(debug) << argv[0] ;
    LOG(info) << argv[0] ;
    LOG(warning) << argv[0] ;
    LOG(error) << argv[0] ;
    LOG(fatal) << argv[0] ;

    return 0 ; 
}
