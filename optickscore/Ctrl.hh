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
#include "OKCORE_API_EXPORT.hh"




struct OKCORE_API Ctrl 
{
    union u_f4_c16
    {
         float f[4] ;
         char  c[16] ;
    };

    Ctrl(float* ptr, unsigned n=4); 
    std::string getCommands() const ; 

    u_f4_c16 fc ; 

    std::vector<std::string> cmds ;

    unsigned num_cmds ; 

};




 
