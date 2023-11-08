#!/usr/bin/env python
"""
oktar.py  
==========

The common prefix includes the trailing slash::

   ...
   Opticks-0.0.0_alpha/x86_64-centos7-gcc48-geant4_10_04_p02-dbg/cmake/Modules/FindPLog.cmake
   Opticks-0.0.0_alpha/x86_64-centos7-gcc48-geant4_10_04_p02-dbg/cmake/Modules/FindGLM.cmake

   Opticks-0.0.0_alpha/x86_64-centos7-gcc48-geant4_10_04_p02-dbg/ 3279 


::

    cd /usr/local/opticks
    ~/opticks/bin/oktar.py /tmp/tt/a.tar --create --prefix Opticks-0.0.1_alpha/i386-10.13.6-gcc4.2.1-geant4_10_04_p02-dbg 


"""
import os, logging, sys, tarfile, argparse, shutil, textwrap
log = logging.getLogger(__name__)

class OKTar(object):
    """
    BASES lists prefix relative directories in order to 
    control the starting points for the recursive adding to the archive

    They allow lots of irrelevant directories from the externals 
    to be excluded from the archive by carefully selecting just what 
    is needed. 
    """
    PREFIX = "Opticks-0.0.1_alpha/i386-10.13.6-gcc4.2.1-geant4_10_04_p02-dbg"
    BASES = filter(None,textwrap.dedent(r"""
    metadata
    bin
    lib
    lib64
    ptx
    include
    cmake
    py
    tests
    externals/include/nljson
    #externals/include/ImGui
    #externals/include/GL
    #externals/include/GLFW
    #externals/imgui/imgui
    externals/imgui/imgui/extra_fonts
    externals/lib
    externals/lib64
    externals/plog/include
    externals/glm/glm/glm
    externals/share/bcm
    """).split("\n"))

    @classmethod
    def Bases(cls):
       return list(filter(lambda _:not _[0] == "#", cls.BASES))  

    @classmethod
    def CreateMode(cls, name):
        mode = None
        if name.endswith(".tar.gz"): 
            mode = "w:gz"
        elif name.endswith(".tar"):
            mode = "w"
        pass
        assert not mode is None, "expecting name ending .tar.gz or .tar : not %s " % (name)
        return mode 


    @classmethod
    def GetSize(self, path):
        if os.path.islink(path):
            sz = 0 
        else: 
            st = os.stat(path) 
            sz = st.st_size
        pass
        return sz

    def __init__(self, path):
        """
        :param path: to the tarfile to open 
        """
        self.path = os.path.expanduser(path)
        self.name = os.path.basename(path)
        self.stem = os.path.splitext(self.name)[0]

        self.t = None
        self.n = []
        self.pfx = "?"
        self.sztot = 0 
        self.sz = {}

        self.exclude_dir_names = [".git",] 

    def dump(self):
        self.t = tarfile.open(self.path, "r")
        for mi in self.t.getmembers():
            sz = mi.size/1e6
            if sz < 1.: continue
            print(" %10.3f : %s " % ( sz, mi.name))
        pass   

    def create(self, prefix):
        """
        :param prefix: relative root for all paths added to archive 
        """
        pass

        elem = prefix.split("/")
        assert len(elem) == 2, "expecting prefix with two path elements : %s " % prefix
        xstem = elem[0]
        assert self.stem == xstem, "tarball name stem [%s] must match first elem of prefix [%s]" % (self.stem, xstem) 

        mode = self.CreateMode(self.name) 
        base = os.path.realpath(os.getcwd())


        outdir = os.path.dirname(self.path)
        if not os.path.isdir(outdir):
            log.info("creating outdir %s " % outdir)
            os.makedirs(outdir)
        pass
        log.info("prefix %s base %s " % (prefix, base))
        log.info("writing path %s mode %s " % (self.path, mode) )

        self.prefix = prefix
        self.t = tarfile.open(self.path, mode)    

        for name in self.Bases():
            path = os.path.join(base, name)
            if not os.path.exists(path): continue
            assert os.path.isdir(path), "expecting only directories at top level of archive"
            self.recurse_(name, 0) 
        pass

    def recurse_(self, relbase, depth ):
        """
        :param relbase: base relative path eg starting with "lib", "include" etc..  
 
        Recurse the tree calling self.add for all paths that 
        are not excluded.
        """
        assert os.path.isdir(relbase), "expected directory %s does not exist " % relbase
        log.debug("relbase %s depth %s " % (relbase, depth))

        names = os.listdir(relbase)
        for name in names:
            relpath = os.path.join(relbase, name)
            if os.path.isdir(relpath):
                 exclude = name in self.exclude_dir_names
                 if not exclude:
                     self.recurse_(relpath, depth+1)
                 pass
            else:
                 self.add(relpath)
            pass
        pass

    def add(self, relpath):
        """
        :param relpath: relative to invoking directory
        """
        arcname = os.path.join(self.prefix,relpath) 
        #log.info("arcname: %s " % arcname )

        self.t.add(relpath, arcname=arcname, recursive=False)

        sz = self.GetSize(relpath)
        self.sztot += sz 
        self.sz[relpath] = sz 

        if sz > 1e6:
            print(" %10.3f : %10.3f M : %s " % ( self.sztot/1e6, sz/1e6, relpath )) 
        pass

    def extract(self, base):
        """
        :param base: directory in which to extract from archive 

        Note that because of the enforced used of a two level common prefix
        its no problem to extract into the same directory as the creation
        """
        self.t = tarfile.open(self.path, "r")
        self.n = self.t.getnames()

        pfx = os.path.commonprefix(self.n)
        assert pfx[-1] == os.sep, ("expect a trailing slash on common prefix", pfx)
        pfx = pfx[:-1] 
        elem = pfx.split(os.sep)
        assert len(elem) == 2, elem  
        assert self.stem == elem[0], ( self.stem, elem[0], "stem of tarball name must match the top level prefix element inside" )
        self.pfx = pfx 

        if not os.path.isdir(base):
            log.info("creating base %s " % base)
            os.makedirs(base)
        pass
        xdir = os.path.join(base, pfx)
        if os.path.isdir(xdir):
            log.info("common prefix extraction dir exists already %s " % xdir)
            log.info("removing xdir %s " % xdir )
            shutil.rmtree(xdir) 
        pass    
        log.info("extracting tarball with common prefix %s into base %s " % (self.pfx, base)) 
        self.t.extractall(base) 
        pass  

    def __str__(self):
        return "\n".join(self.n)
    def __repr__(self):
        return "OKTar %s %d " % ( self.pfx, len(self.n)) 

if __name__ == '__main__':

    parser = argparse.ArgumentParser(__doc__)

    base = "/tmp/cvmfs/opticks.ihep.ac.cn/ok/releases"
    prefix = OKTar.PREFIX

    parser.add_argument( "path",  nargs=1, help="Path of distribution tarball, eg ~/Opticks-0.0.0_alpha.tar " )
    parser.add_argument( "mode", choices=["create","extract","dump"] )

    parser.add_argument( "--base",  default=os.getcwd(), help="Path at which to explode tarballs %(default)s ")   
    parser.add_argument( "--prefix", default=prefix, help="Prefix to all paths that will be added to archive" ) 

    desc = { 'extract':"Extract tarball contents into base" , 
             'create':"Create with contents of current directory or --base argument if specified",
             'dump':"Dump names and sizes of tarball members" }

    #parser.add_argument( "--extract", action="store_true", default=False, help="Extract tarball contents into base ")   
    #parser.add_argument( "--dump",    action="store_true", default=False, help="Dump names and sizes of tarball members ")   
    #parser.add_argument( "--create",  action="store_true", default=False, help="Create with contents of current directory ")   

    parser.add_argument( "--level", default="info", help="logging level" ) 
    args = parser.parse_args()

    fmt = '[%(asctime)s] p%(process)s {%(pathname)s:%(lineno)d} %(levelname)s - %(message)s'
    logging.basicConfig(level=getattr(logging,args.level.upper()), format=fmt)


    t = OKTar(args.path[0])

    if args.mode == "dump": 
        t.dump()
    elif args.mode == "create":
        t.create(args.prefix)
    elif args.mode == "extract": 
        t.extract(args.base)
    pass



