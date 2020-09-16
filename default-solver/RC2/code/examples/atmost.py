#!/usr/bin/env python
#-*- coding:utf-8 -*-
##
## atmost.py
##
##  Created on: May 15, 2018
##      Author: Alexey S. Ignatiev
##      E-mail: aignatiev@ciencias.ulisboa.pt
##

#
#==============================================================================
from __future__ import print_function
import getopt
import os
from pysat.card import *
from pysat.formula import WCNF
from six.moves import range
import sys


# cardinality encodings
#==============================================================================
encmap = {
    'seqc': EncType.seqcounter,
    'cardn': EncType.cardnetwrk,
    'sortn': EncType.sortnetwrk,
    'tot': EncType.totalizer,
    'mtot': EncType.mtotalizer,
    'kmtot': EncType.kmtotalizer
}


#
#==============================================================================
class AtMostK(WCNF, object):
    """
        Generates a partial MaxSAT formula whose hard part enforces at most k
        literals out of m to be true while the soft part prefers all of them
        to be true.
    """

    def __init__(self, mval=8, kval=1, enc=EncType.kmtotalizer):
        """
            Constructor.
        """

        # initializing WCNF's internal variables
        super(AtMostK, self).__init__()

        # literals of the left-hand side
        lhs = list(range(1, mval + 1))

        # hard clauses
        hard = CardEnc.atmost(lhs, bound=kval, top_id=mval, encoding=enc)
        for cl in hard.clauses:
            self.hard.append(cl)

        # soft clauses
        for l in lhs:
            self.soft.append([l])
            self.wght.append(1)

        # number of variables
        self.nv = hard.nv

        # top weight
        self.topw = len(self.soft) + 1


#
#==============================================================================
def parse_options():
    """
        Parses command-line options:
    """

    try:
        opts, args = getopt.getopt(sys.argv[1:], 'e:hk:m:',
                ['enc=', 'help', 'kval=', 'mval='])
    except getopt.GetoptError as err:
        sys.stderr.write(str(err).capitalize())
        usage()
        sys.exit(1)

    enc = 'seqc'
    kval = 1
    mval = 8

    for opt, arg in opts:
        if opt in ('-e', '--enc'):
            enc = str(arg)
        elif opt in ('-h', '--help'):
            usage()
            sys.exit(0)
        elif opt in ('-k', '--kval'):
            kval = int(arg)
        elif opt in ('-m', '--mval'):
            mval = int(arg)
        else:
            assert False, 'Unhandled option: {0} {1}'.format(opt, arg)

    enc = encmap[enc]
    return enc, kval, mval


#
#==============================================================================
def usage():
    """
        Prints usage message.
    """

    print('Usage:', os.path.basename(sys.argv[0]), '[options]')
    print('Options:')
    print('        -e, --enc=<string>    Cardinality encoding')
    print('                              Available values: cardn, kmtot, mtot, seqc, sortn, tot (default = kmtot)')
    print('        -h, --help')
    print('        -k, --kval=<int>      Right-hand side (k)')
    print('                              Available values: [1 .. INT_MAX] (default = 1)')
    print('        -m, --mval=<int>      Size of the left-hand size (m)')
    print('                              Available values: [0 .. INT_MAX] (default = 8)')

#
#==============================================================================
if __name__ == '__main__':
    enc, kval, mval = parse_options()

    wcnf = AtMostK(mval, kval, enc)
    wcnf.to_fp(sys.stdout)
