#!/bin/bash -l 

defarg="run"
arg=${1:-$defarg}

bin=U4NavigatorTest

export GEOM=J004G
export J004G_GDMLPath=$HOME/.opticks/GEOM/J004/origin.gdml

if [ "${arg/run}" != "$arg" ]; then 
    $bin
    [ $? -ne 0 ] && echo $BASH_SOURCE run error && exit 1 
fi 

if [ "${arg/dbg}" != "$arg" ]; then 

    case $(uname) in 
    Darwin) lldb__ $bin ;;
    Linux)  gdb__ $bin ;;
    esac

    [ $? -ne 0 ] && echo $BASH_SOURCE dbg error && exit 2
fi 

exit 0 




