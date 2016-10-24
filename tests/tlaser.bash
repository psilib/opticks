tlaser-source(){   echo $(opticks-home)/tests/tlaser.bash ; }
tlaser-vi(){       vi $(tlaser-source) ; }
tlaser-usage(){ cat << \EOU

tlaser- : Pencil beam 
==================================================

See :doc:`notes/issues/geant4_opticks_integration/reemission_review`


EOU
}
tlaser-env(){      olocal- ;  }
tlaser-dir(){ echo $(opticks-home)/tests ; }
tlaser-cd(){  cd $(tlaser-dir); }

join(){ local IFS="$1"; shift; echo "$*"; }

tlaser-tag(){ echo 1 ; }

tlaser--(){

    local msg="=== $FUNCNAME :"

    #local photons=1000000
    #local photons=20000
    local photons=100000
    #local photons=10000

    local g4ppe=10000  # default 10k photons per g4 evt (subevt splitting for G4 memory reasons)
    #local g4ppe=$photons

    local tag=$(tlaser-tag)
    local torch_config=(
                 type=point
                 frame=3153
                 source=0,0,0
                 target=1,0,0
                 photons=$photons
                 material=GdDopedLS
                 wavelength=430
                 weight=1.0
                 time=0.1
                 zenithazimuth=0,1,0,1
                 radius=0
               )

    op.sh  \
            $* \
            --g4ppe $g4ppe \
            --animtimemax 15 \
            --timemax 15 \
            --eye 0,1,0 \
            --torch --torchconfig "$(join _ ${torch_config[@]})" \
            --save --tag $tag --cat laser



}
 
tlaser-args(){ echo --tag $(tlaser-tag) --det laser --src torch ; } 
tlaser-i(){ ipython -i $(which tlaser.py) ; }


tlaser-t(){ tlaser-;tlaser-- --okg4 --compute $* ; } 
tlaser-d(){ tlaser-;tlaser-t --steppingdbg  ; } 


