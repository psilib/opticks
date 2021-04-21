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

/**
SPath
======

**/

#include "SYSRAP_API_EXPORT.hh"

class SYSRAP_API SPath {
  public:
      static const char* Stem( const char* name );
      static bool IsReadable(const char* path);
      static const char* GetHomePath(const char* rel); 
      static const char* Dirname(const char* path); 
      static const char* Basename(const char* path); 
      static const char* UserTmpDir(const char* pfx="/tmp", const char* user_envvar="USER", const char* sub="opticks", char sep='/'  );
      static const char* Resolve(const char* path); 
      static const char* Resolve(const char* dir, const char* name);
      static const char* Resolve(const char* dir, const char* reldir, const char* name);
};



