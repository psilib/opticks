#!/usr/bin/env python
"""
::

    ggeo.py --mmtrim    # create list of mm names used for labels at $TMP/mm.txt

    snap.py       # list the snaps in speed order with labels 

    open $(snap.py --jpg)         # open the jpg ordered by render speed


"""
import os, sys, logging, glob, json, re, argparse 
log = logging.getLogger(__name__)
import numpy as np
from opticks.ana.rsttable import RSTTable
from opticks.CSG.CSGFoundry import CSGFoundry, LV, MM


class MM(object):
    """
    The input file is now a standard resident of the CSGFoundry directory 
    Note that the input file was formerly created using::

       ggeo.py --mmtrim > $TMP/mm.txt

    """
    PTN = re.compile("\d+") 
    def __init__(self, path ):
        mm = os.path.expandvars(path)
        mm = open(mm, "r").read().splitlines() if os.path.exists(mm) else None
        self.mm = mm
        if mm is None:
            log.fatal("missing %s, which is now a standard part of CSGFoundry " % path  ) 
            sys.exit(1)
        pass

    def imm(self, emm):
        return list(map(int, self.PTN.findall(emm))) 

    def label(self, emm):
        imm = self.imm(emm)
        labs = [self.mm[i] for i in imm] 
        lab = " ".join(labs)

        tilde = emm[0] == "t" or emm[0] == "~"
        pfx = (  "NOT: " if tilde else "     " ) 

        if emm == "~0" or emm == "t0":
            return "ALL"
        elif imm == [1,2,3,4]:
            return "ONLY PMT"
        elif "," in emm:
            return ( "EXCL: " if tilde else "ONLY: " ) +  lab 
        else:
            return lab  
        pass

    def __repr__(self):
        return "\n".join(self.mm)


class DummyCandleSnap(object):
    def __init__(self):
        self.av = 1 


class Snap(object):
    PTN = re.compile("cxr_overview_emm_(?P<emm>\S*)_elv_(?P<elv>\S*)_moi_(?P<moi>\S*)")

    @classmethod
    def is_valid(cls, jpg_path):
        #json_path = cls.find_json(jpg_path) 
        json_path = jpg_path.replace(".jpg", ".json")
        valid = (not json_path is None) and json_path[-5:] == ".json"
        log.debug("is_valid %r %r %r " % (jpg_path, json_path, valid ) ) 
        return valid 

    @classmethod
    def ParseStem(cls, jpg_stem):   
        m = cls.PTN.match(jpg_stem)
        return m.groupdict() if not m is None else {}

    def __init__(self, jpg_path):
        json_path = jpg_path.replace(".jpg", ".json")
        jpg_stem = os.path.splitext(os.path.basename(jpg_path))[0]
        log.debug("jpg_path %s json_path %s " % (jpg_path, json_path))          
        js = json.load(open(json_path,"r"))

        self.js = js 
        self.jpg = jpg_path
        self.path = json_path 
        self.av = js['av'] 
        self.argline = js['argline']

        dstem = self.ParseStem(jpg_stem)

        elv = dstem.get("elv", None)
        emm = dstem.get("emm", None)

        if not emm is None:
            assert js["emm"] == emm
        pass 

        self.elv = elv
        self.emm = emm 

        if not elv is None:
            enabled = elv
        else:
            enabled = emm
        pass
        self.enabled = enabled

        # below are set by SnapScan after sorting
        self.sc = None   
        self.idx = None  

    def jpg_(self):
        """
        tilde ~ and __t0__ causing RST problems : so replace tham 
        """
        fold = os.path.dirname(self.jpg)
        tname = self.jpg_tname()
        return os.path.join(fold, tname)

    def jpg_tname(self): 
        """
        This is a workaround for problems with s5 machinery for names 
        containing strings like the below, presumably due to some RST meaning 
        the names get mangled preventing the association between presentation pages and 
        background image definition causes the images to not appear::

            __~0__
            __t0__
            _ALL_

        ACTUALLY THIS ISSUE MAY BE FROM NON-UNIQUE IDENTIFIERS DERIVED FROM THE 
        SLIDE TITLES BY REMOVING SOME CHARS SUCH AS "," "_" "~"
        """
        name = os.path.basename(self.jpg)
        tname = name.replace("~","t")
        tname = tname.replace("__t0__", "_all_") 
        return tname 

    def mvjpg(self):
        name = os.path.basename(self.jpg)
        tname = self.jpg_tname()
        return None if name == tname else "mv %s %s" % (name, tname)

    def cpjpg(self, pfx, s5base):
        s5base = os.path.expandvars(s5base)
        name = os.path.basename(self.jpg)
        ppath = "%s%s/%s"%(s5base,pfx,name)
        if os.path.exists(ppath):
            ret = None   
        else:
            ret = "cp %s %s" % (self.jpg, ppath)
        return ret 

    def refjpg(self, pfx, afx="1280px_720px", indent="    "): 
        """
        For inclusion into s5_background_image.txt 
        """
        name = os.path.basename(self.jpg_())
        return "\n".join([indent+self.title(), indent+pfx+"/"+name+" "+afx, ""])

    def title(self):
        name = os.path.basename(self.jpg_())
        stem = name.replace(".jpg","") 
        return "[%d]%s" % (self.idx, stem)
 
    def pagetitle(self,kls="blue"):
        return ":%s:`%s`" % (kls,self.title())
 
    def pagejpg(self):
        title = self.pagetitle()
        return "\n".join([title, "-" * len(title), ""])


    over_fast = property(lambda self:float(self.av)/float(self.sc.fast.av))
    over_slow = property(lambda self:float(self.av)/float(self.sc.slow.av))
    over_candle = property(lambda self:float(self.av)/float(self.sc.candle.av))

    label = property(lambda self:self.sc.label(self.enabled))
    imm = property(lambda self:self.sc.mm.imm(self.emm))

    def row(self):
        return  (int(self.idx), self.enabled, self.av, self.over_candle, self.label )


    SPACER = "    "
    LABELS = ["idx", "-e", "time(s)", "relative", "enabled geometry description" ] 
    WIDS = [     3,     10,     10,        10,       70 ] 
    HFMT = [  "%3s",  "%10s", "%10s",   "%10s",   "%-70s" ]  
    RFMT = [  "%3d", "%10s", "%10.4f", "%10.4f",  "%-70s" ]  
    PRE  =  [  ""   , ""    , SPACER , SPACER  ,  SPACER ] 
    POST =  [  ""   , ""    , SPACER , SPACER  ,  SPACER ]  


    def __repr__(self):
        RFMT = [ self.PRE[i] + self.RFMT[i] +self.POST[i] for i in range(len(self.RFMT))]
        rfmt = " ".join(RFMT)
        #print(rfmt)
        return rfmt % self.row()

    @classmethod
    def Hdr(cls):
        HFMT = [cls.PRE[i] + cls.HFMT[i] + cls.POST[i] for i in range(len(cls.HFMT))]
        hfmt = " ".join(HFMT)
        labels = cls.LABELS
        return hfmt % tuple(labels)


class SnapScan(object):
    @classmethod
    def MakeSnaps(cls, globptn):
        """
        * resolve the globptn into a sorted list of paths, typically of jpg renders
        * order is based on Snap.av obtained from the sidecar json file
        """ 
        log.info("globptn %s " % globptn )
        raw_paths = glob.glob(globptn) 
        log.info("raw_paths %d : 1st %s " % (len(raw_paths), raw_paths[0]))
        paths = filter(lambda p:Snap.is_valid(p), raw_paths)  # seems all paths are for now valid
        snaps = list(map(Snap,paths))
        snaps = sorted(snaps, key=lambda s:s.av)
        return snaps

    @classmethod
    def SelectSnaps(cls, all_snaps ):
        snaps = []
        for s in all_snaps:  
            #print(s.imm)
            if len(s.imm) == 1 or s.imm == [8, 0] or s.imm == [1,2,3,4]:
                snaps.append(s)
            else:
                log.debug("skip %s " % str(s.imm))
            pass   
        pass
        return snaps

    @classmethod
    def Create(cls, globptn):
        base = os.path.expandvars(os.path.dirname(globptn)) 
        cfdigest = CSGFoundry.FindDigest(base)
        cfdir = CSGFoundry.FindDirUpTree(base)
        log.info("cfdir %s cfdigest %s " % (cfdir, cfdigest) ) 

        mmlabel_path = os.path.join(cfdir, "mmlabel.txt")
        log.info("mmlabel_path %s " % mmlabel_path ) 
        mm = MM(mmlabel_path)

        meshname_path = os.path.join(cfdir, "meshname.txt")
        log.info("meshname_path %s " % meshname_path ) 
        lv = LV(meshname_path)

        elv_mode = globptn.find("elv") > -1
        sc = cls(globptn, mm, lv, elv_mode=elv_mode, cfdigest=cfdigest, cfdir=cfdir  )
        return sc 


    def __init__(self, globptn, mm=None, lv=None, candle_emm="1,2,3,4", elv_mode=False, cfdigest=None, cfdir=None):
        """
        :param globptn: eg 
       
        /Users/blyth/.opticks/geocache/DetSim0Svc_pWorld_g4live/g4ok_gltf/41c046fe05b28cb70b1fc65d0e6b7749/1/CSG_GGeo/CSGOptiXRenderTest/
            cvd1/70000/cxr_overview/cam_0_tmin_0.4/cxr_overview*.jpg
 
        """
        self.mm = mm
        self.lv = lv
        self.elv_mode = elv_mode
        self.cfdigest = cfdigest
        self.cfdir = cfdir 

        all_snaps = self.MakeSnaps(globptn)
        candle = None
        for s in all_snaps:
            s.sc = self
            if s.emm == candle_emm:
                candle = s
            pass  
        pass

        # filter out the double emm
        #snaps = list(filter(lambda s:len(s.imm) != 2, snaps))
        snaps = self.SelectSnaps(all_snaps)

        for idx, s in enumerate(snaps):
            s.idx = idx
        pass
        self.snaps = snaps

        if candle is None:
            candle = DummyCandleSnap()
        pass  
        self.candle = candle 

    def label(self, enabled):
        return self.lv.label(enabled) if self.elv_mode else self.mm.label(enabled)

    def table(self):
        table = np.empty([len(self.snaps),len(Snap.LABELS)], dtype=np.object ) 
        for idx, snap in enumerate(self.snaps):
            table[idx] = snap.row()
        pass
        return table

    def rst_table(self):
        t = self.table()

        labels = Snap.LABELS 
        labels[-1] += " " + self.cfdigest[:8]

        rst = RSTTable.Render(t, labels, Snap.WIDS, Snap.HFMT, Snap.RFMT, Snap.PRE, Snap.POST )
        return rst 


    fast = property(lambda self:self.snaps[0])
    slow = property(lambda self:self.snaps[-1])

    def __repr__(self):
        return "\n".join( [Snap.Hdr()] + list(map(repr,self.snaps)) + [Snap.Hdr()] )

    def jpg(self):
        return "\n".join(list(map(lambda s:s.jpg,self.snaps)))

    def mvjpg(self):
        return "\n".join(list(filter(None,map(lambda s:s.mvjpg(),self.snaps))))

    def cpjpg(self, pfx, s5base):
        return "\n".join(list(filter(None,map(lambda s:s.cpjpg(pfx, s5base),self.snaps))))

    def argline(self):
        return "\n".join(list(map(lambda s:s.argline,self.snaps)))

    def refjpg(self, pfx): 
        return "\n".join(list(map(lambda s:s.refjpg(pfx),self.snaps)))

    def pagejpg(self): 
        return "\n".join(list(map(lambda s:s.pagejpg(),self.snaps)))

        
def parse_args(doc, **kwa):
    np.set_printoptions(suppress=True, precision=3, linewidth=200)
    parser = argparse.ArgumentParser(doc)
    parser.add_argument(  "--level", default="info", help="logging level" ) 
    parser.add_argument(  "--globptn", default="$TMP/snap/*.jpg", help="base" ) 
    parser.add_argument(  "--jpg", action="store_true", help="List jpg paths in speed order" ) 
    parser.add_argument(  "--refjpgpfx", default="/env/presentation/snap/lLowerChimney_phys", help="List jpg paths s5 background image presentation format" ) 
    parser.add_argument(  "--s5base", default="$HOME/simoncblyth.bitbucket.io", help="Presentation repo base" )
    parser.add_argument(  "--refjpg", action="store_true", help="List jpg paths s5 background image presentation format" ) 
    parser.add_argument(  "--pagejpg", action="store_true", help="List jpg for inclusion into s5 presentation" ) 
    parser.add_argument(  "--mvjpg", action="store_true", help="List jpg for inclusion into s5 presentation" ) 
    parser.add_argument(  "--cpjpg", action="store_true", help="List cp commands to place into presentation repo" ) 
    parser.add_argument(  "--argline", action="store_true", help="List argline in speed order" ) 
    parser.add_argument(  "--rst", action="store_true", help="Dump table in RST format" ) 
    parser.add_argument(  "--snaps", action="store_true", help="Debug: just create SnapScan " ) 
    args = parser.parse_args()
    fmt = '[%(asctime)s] p%(process)s {%(pathname)s:%(lineno)d} %(levelname)s - %(message)s'
    logging.basicConfig(level=getattr(logging,args.level.upper()), format=fmt)
    return args  


if __name__ == '__main__':
    args = parse_args(__doc__)
    log.debug(" args %s " % str(args))

    globptn = args.globptn
    log.info("globptn %s " % (globptn) ) 

    
    ss = SnapScan.Create(args.globptn) 
    if args.jpg:
        print(ss.jpg())
    elif args.refjpg:
        print(ss.refjpg(args.refjpgpfx))
    elif args.pagejpg:
        print(ss.pagejpg())
    elif args.mvjpg:
        print(ss.mvjpg())
    elif args.cpjpg:
        print(ss.cpjpg(args.refjpgpfx, args.s5base))
    elif args.argline:
        print(ss.argline())
    elif args.snaps:
        snaps = ss.snaps
    elif args.rst:
        print(ss.rst_table())
    else:
        print(ss) 
    pass



