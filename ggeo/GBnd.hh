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

#pragma once

#include <string>
#include <vector>

#include "GGEO_API_EXPORT.hh"
#include "GGEO_HEAD.hh"

class GMaterialLib ; 
class GSurfaceLib ; 

struct GGEO_API GBnd
{
    GBnd(const char* spec_, bool flip_, GMaterialLib* mlib_, GSurfaceLib* slib_, bool dbgbnd_ ) ;
    void init(bool flip_);
    void check();

    unsigned           UNSET ; 
    const char*         spec ; 
    GMaterialLib*       mlib ;    // getIndex may trigger a close, so cannot be const 
    GSurfaceLib*        slib ; 
    bool                dbgbnd ; 


    const char* omat_ ; 
    const char* osur_ ; 
    const char* isur_ ; 
    const char* imat_ ;

    unsigned  omat ; 
    unsigned  osur ; 
    unsigned  isur ; 
    unsigned  imat ; 

    bool has_osur() const ; 
    bool has_isur() const ; 


    std::vector<std::string> elem ; 
};

#include "GGEO_TAIL.hh"

