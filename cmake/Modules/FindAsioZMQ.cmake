#include(OPTICKSCfg)

set(AsioZMQ_PREFIX "${OPTICKS_PREFIX}/network/asiozmq")
set(AsioZMQ_LIBRARIES "" )
set(AsioZMQ_INCLUDE_DIRS "${AsioZMQ_PREFIX}/include")
set(AsioZMQ_DEFINITIONS "-DBOOST_ASIO_HAS_STD_CHRONO")

