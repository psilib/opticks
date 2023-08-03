#!/bin/bash -l 

name=stamp_test 
bin=/tmp/$name
export TTPATH=/tmp/$name.npy

#defarg="build_run_ana"
defarg="build_run"
arg=${1:-$defarg}

if [ "${arg/build}" != "$arg" ]; then 
    gcc $name.cc -std=c++11 -lstdc++ -I$HOME/np -I.. -o $bin
    [ $? -ne 0 ] && echo $BASH_SOURCE build error && exit 1 
fi 

if [ "${arg/run}" != "$arg" ]; then 
    $bin
    [ $? -ne 0 ] && echo $BASH_SOURCE run  error && exit 2
fi 

if [ "${arg/ana}" != "$arg" ]; then 
    ${IPYTHON:-ipython} --pdb -i $name.py 
    [ $? -ne 0 ] && echo $BASH_SOURCE ana  error && exit 3
fi 

exit 0 

