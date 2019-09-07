#!/usr/bin/env python
#
# Copyright (c) 2019 Opticks Team. All Rights Reserved.
#
# This file is part of Opticks
# (see https://bitbucket.org/simoncblyth/opticks).
#
# Licensed under the Apache License, Version 2.0 (the "License"); 
# you may not use this file except in compliance with the License.  
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software 
# distributed under the License is distributed on an "AS IS" BASIS, 
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
# See the License for the specific language governing permissions and 
# limitations under the License.
#


import os, numpy as np

np.set_printoptions(precision=8, suppress=True)

if __name__ == '__main__':
    IBASE = os.environ.get("TRngBuf_IBASE", 0) 
    a = np.load(os.path.expandvars("$TMP/TRngBufTest_%d.npy" % IBASE))
    print(a.shape)
    print(a)
    print(a.shape)
    print(a.dtype)



