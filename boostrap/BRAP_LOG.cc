
#include <plog/Log.h>

#include "BRAP_LOG.hh"
#include "PLOG_INIT.hh"
#include "PLOG.hh"
       
void BRAP_LOG::Initialize(void* whatever, int level )
{
    PLOG_INIT(whatever, level);
}
void BRAP_LOG::Check(const char* msg)
{
    PLOG_CHECK(msg);
}

