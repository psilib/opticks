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

#include "Randomize.hh"

struct MyEngine : public CLHEP::MixMaxRng 
{
    double flat(){ return .42 ; }
};

int main(int argc, char** argv)
{
    if(argc > 1) CLHEP::HepRandom::setTheEngine(new MyEngine()); 

    for( unsigned i=0 ; i < 10 ; i++)
    {
        double u = G4UniformRand(); 
        std::cout << u << std::endl ; 
    }

    return 0 ; 
}

