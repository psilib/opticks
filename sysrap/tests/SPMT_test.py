#!/usr/bin/env python
"""


SPMT pmtcat order is by numerical enum value 000,001,003::

    006 enum PMT_CATEGORY {
      7   kPMT_Unknown=-1,
      8   kPMT_NNVT,          # 000
      9   kPMT_Hamamatsu,     # 001
     10   kPMT_HZC,
     11   kPMT_NNVT_HighQE    # 003
     12 };


JPMT order is HAMA, NNVT, NNVTMCP_HiQE::

    In [2]: s.jpmt_rindex_names
    Out[2]: 
    R12860
    NNVTMCP
    NNVTMCP_HiQE


Comparing JPMT with SPMT see ordering flip 0<->1 and 1e3 scale difference::

    In [13]:  s.jpmt_thickness.T
    Out[13]: 
    array([[[ 0.  ,  0.  ,  0.  ],
            [36.49, 40.  , 10.24],
            [21.13, 20.58, 18.73],
            [ 0.  ,  0.  ,  0.  ]]])

    In [14]: t.thickness.T
    Out[14]: 
    array([[[    0.,     0.,     0.],
            [40000., 36490., 10240.],
            [20580., 21130., 18730.],
            [    0.,     0.,     0.]]])


    In [10]: np.allclose( a[0],b[1] )
    Out[10]: True

    In [11]: np.allclose( a[1],b[0] )
    Out[11]: True

    In [9]: np.allclose( a[2],b[2] )
    Out[9]: True


After JPMT.h reordering and scale fixes::

    In [6]: np.allclose( a, b )
    Out[6]: True

    In [7]: np.abs(a-b).max()
    Out[7]: 4.440892098500626e-16

    In [5]: np.abs( s.jpmt_thickness - t.thickness ).max()
    Out[5]: 3.552713678800501e-15


"""
import numpy as np
from opticks.ana.fold import Fold

if __name__ == '__main__':
    t = Fold.Load(symbol="t")
    print(repr(t))

    s = Fold.Load("/tmp/JPMTTest", symbol="s")
    print(repr(s))

    a = t.rindex
    b = s.jpmt_rindex

    #expr = "a.reshape(-1,2)[:,0]"
    #print(expr) 
    #print(eval(expr)) 
pass

