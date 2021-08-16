#!/usr/bin/env python
"""
::
 
    ipython -i QRngTest.py 


When using skipaheadstep of 1::


   In [21]: np.all( uu[1,:,:-1] == uu[0,:,1:] )
   Out[21]: True


"""
import logging 
log = logging.getLogger(__name__)
import matplotlib.pyplot as plt 
import os, numpy as np


class QRngTest(object):
    FOLD = "/tmp/QRngTest"
    def __init__(self, reldir):
        base = os.path.join(self.FOLD, reldir)
        uu = np.load(os.path.join(base, "uu.npy"))
        dtype = uu.dtype

        if reldir == "float":
            assert dtype == np.float32
        elif reldir == "double":
            assert dtype == np.float64
        else:
            assert 0, "unexpected reldir %s " % reldir
        pass
        self.reldir = reldir
        self.dtype = dtype
        self.uu = uu
        self.h = np.histogram( self.uu )
        self.title = "qudarap/tests/QRngTest.py %s " % base


    def check_skipahead_shifts(self, offset):
        uu = self.uu
        assert len(uu.shape) == 3
        ni, nj, nk = uu.shape 
        for i in range(ni-1):
            i0 = i
            i1 = i+1
            assert np.all( uu[i1,:,:-offset] == uu[i0,:,offset:] )
            log.info("i0 %d i1 %d " % (i0,i1)) 
        pass

    def plot(self):
        t = self
        h = self.h 

        fig, ax = plt.subplots(figsize=[12.8,7.2])
        fig.suptitle(self.title)

        ax.plot( h[1][:-1], h[0], label="h", drawstyle="steps-post" )
        ax.set_ylim( 0, h[0].max()*1.1 )

        pass
        ax.legend()
        fig.show()
        path = os.path.join(t.FOLD, self.reldir, "fig.png")
        log.info("save to %s " % path)
        fig.savefig(path)



if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)

    reldir = "float"
    #reldir = "double"
    t = QRngTest(reldir)  
    t.plot()
    uu = t.uu
    h = t.h

    t.check_skipahead_shifts(1)


