##
## Copyright (c) 2019 Opticks Team. All Rights Reserved.
##
## This file is part of Opticks
## (see https://bitbucket.org/simoncblyth/opticks).
##
## Licensed under the Apache License, Version 2.0 (the "License"); 
## you may not use this file except in compliance with the License.  
## You may obtain a copy of the License at
##
##   http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software 
## distributed under the License is distributed on an "AS IS" BASIS, 
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
## See the License for the specific language governing permissions and 
## limitations under the License.
##

sysrap-src(){      echo sysrap/sysrap.bash ; }
sysrap-source(){   echo ${BASH_SOURCE:-$(opticks-home)/$(sysrap-src)} ; }
sysrap-vi(){       vi $(sysrap-source) ; }
sysrap-usage(){ cat << \EOU

System Rap
============

Lowest level package, beneath BoostRap and 
**explicitly not using Boost**. 

A lower level pkg that BoostRap is required 
as nvcc, the CUDA compiler, has trouble compiling 
some Boost headers.

Linux
-------

::

    yum install openssl-devel   # for openssl/md5.h 



EOU
}

sysrap-env(){      olocal- ; opticks- ;  }

sysrap-dir(){  echo $(sysrap-sdir) ; }
sysrap-sdir(){ echo $(opticks-home)/sysrap ; }
sysrap-tdir(){ echo $(opticks-home)/sysrap/tests ; }
sysrap-idir(){ echo $(opticks-idir); }
sysrap-bdir(){ echo $(opticks-bdir)/sysrap ; }

sysrap-c(){    cd $(sysrap-sdir); }
sysrap-cd(){   cd $(sysrap-sdir); }
sysrap-scd(){  cd $(sysrap-sdir); }
sysrap-tcd(){  cd $(sysrap-tdir); }
sysrap-icd(){  cd $(sysrap-idir); }
sysrap-bcd(){  cd $(sysrap-bdir); }

sysrap-name(){ echo SysRap ; }
sysrap-tag(){  echo SYSRAP ; }

sysrap-apihh(){  echo $(sysrap-sdir)/$(sysrap-tag)_API_EXPORT.hh ; }
sysrap---(){     touch $(sysrap-apihh) ; sysrap--  ; }


sysrap-wipe(){    local bdir=$(sysrap-bdir) ; rm -rf $bdir ; }

#sysrap--(){       opticks-- $(sysrap-bdir) ; } 
#sysrap-t(){       opticks-t $(sysrap-bdir) $* ; } 
sysrap--(){       sysrap-scd ; om- ; om-make ;  } 
sysrap-t(){       sysrap-scd ; om- ; om-test ;  } 




sysrap-ts(){      opticks-ts $(sysrap-bdir) $* ; } 
sysrap-genproj(){ sysrap-scd ; opticks-genproj $(sysrap-name) $(sysrap-tag) ; } 
sysrap-gentest(){ sysrap-tcd ; opticks-gentest ${1:-SCheck} $(sysrap-tag) ; } 
sysrap-txt(){     vi $(sysrap-sdir)/CMakeLists.txt $(sysrap-tdir)/CMakeLists.txt ; } 

sysrap-csg(){ head -20 $(sysrap-dir)/OpticksCSG.h ; }


sysrap-SDigestTest-(){ cat << EOF
0123456789abcdef0123456789abcdef
EOF
}

sysrap-SDigestTest()
{
    local path=/tmp/$FUNCNAME.txt
    $FUNCNAME- > $path
    echo $path 
    ls -l $path

    SDigestTest $path 0  16
    SDigestTest $path 16 32

}



sysrap-csg-generate()
{
    local msg="$FUNCNAME : " 
    local iwd=$PWD
    sysrap-cd
    c_enums_to_python.py OpticksCSG.h 

    echo $msg To write above generated python to OpticksCSG.py ..

    local ans
    read -p "Enter YES ... " ans

    if [  "$ans" == "YES" ]; then 
       c_enums_to_python.py OpticksCSG.h > OpticksCSG.py 

       echo $msg checking the generated python is valid 
       python  OpticksCSG.py

    else
       echo $msg SKIP
    fi 

    cd $iwd
}







sysrap-tags-alphabetical()
{
    local iwd=$(pwd)
    local expo
    cd $(opticks-home) ; ls -1 */*_API_EXPORT.hh  | while read expo ; do 
        local subtag=${expo/_API_EXPORT.hh}
        local sub=$(dirname $subtag)
        local tag=$(basename $subtag)
        #printf "%-20s : %s \n" $sub $tag 
        echo $tag 
    done
    cd $iwd
}

sysrap-tags-ordered()  
{
   opticks-deps --tags 
}

sysrap-tags-foreach()
{
    local func=${1:-echo}
    local tag
    sysrap-tags-ordered | while read tag 
    do
        [ "$tag" == "OKCONF" ] && continue    ## OKCONF is beneath SYSRAP so it has no logging 
        $func $tag
    done
}
sysrap-genlog()
{
   sysrap-cd
   sysrap-genlog-hh > OPTICKS_LOG.hh
}

sysrap-genlog-includes-(){ cat << EOH
#ifdef OPTICKS_$1
#include "$1_LOG.hh"
#endif
EOH
}
sysrap-genlog-initialize-(){ cat << EOI
#ifdef OPTICKS_$1
    $1_LOG::Initialize(instance->prefixlevel_parse( max_level, "$1"), app1, NULL );
#endif
EOI
}
sysrap-genlog-check-(){ cat << EOC
#ifdef OPTICKS_$1
    $1_LOG::Check("$1");
#endif
EOC
}

sysrap-genlog-hh(){ cat << EOI
#pragma once

/**

OPTICKS_LOG
===============

Umbrella logging header bringing in logging headers from all linked Opticks subprojects 
allowing the logging of each subproject to be individually controlled.

The header was generated by $FUNCNAME $(date)

To regenerate the sysrap/OPTICKS_LOG.hh header use commandline::

     sysrap-;sysrap-genlog

**/

$(sysrap-tags-foreach sysrap-genlog-includes-)

#include "SYSRAP_API_EXPORT.hh"

#include "PLOG.hh"
// NB must implement in header as this code is required to live in the main, not in a lib

class SYSRAP_API OPTICKS_LOG_ {
   public:
       // initialize all linked loggers and hookup the main logger
       static void Initialize(PLOG* instance, void* app1, void* app2 )
       {
           int max_level = instance->parse("info") ;  
           // note : can decrease verbosity from the max_level in the subproj, but not increase

$(sysrap-tags-foreach sysrap-genlog-initialize-)
       }
       // checking the loggers in each subproj by emitting message at all loglevels
       static void Check()
       {
$(sysrap-tags-foreach sysrap-genlog-check-)
       }
};

#define OPTICKS_LOG_COLOR__(argc, argv) \
{  \
    PLOG_COLOR(argc, argv); \
    OPTICKS_LOG_::Initialize(PLOG::instance, plog::get(), NULL );  \
}  \

#define OPTICKS_LOG__(argc, argv) \
{  \
    PLOG_(argc, argv); \
    OPTICKS_LOG_::Initialize(PLOG::instance, plog::get(), NULL ); \
} \

#define OPTICKS_LOG(argc, argv) \
{  \
    PLOG_COLOR(argc, argv); \
    OPTICKS_LOG_::Initialize(PLOG::instance, plog::get(), NULL ); \
} \


EOI
}

