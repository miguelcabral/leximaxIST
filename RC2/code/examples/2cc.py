#!/usr/bin/env python
#-*- coding:utf-8 -*-
##
## 2cc.py
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
from pysat.formula import CNF
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
class TwoCC(CNF, object):
    """
        Generates two clashing cardinality constraints.
    """

    def __init__(self, mval=8, kval=1, dval=1, enc=EncType.kmtotalizer):
        """
            Constructor.
        """

        # initializing CNF's internal variables
        super(TwoCC, self).__init__()

        # literals of the left-hand side
        lhs = list(range(1, mval + 1))

        # atmost(k) constraint
        cnf = CardEnc.atmost(lhs, bound=kval, top_id=mval, encoding=enc)
        self.clauses = cnf.clauses
        self.nv = cnf.nv

        # atleast(k + d) constraint
        cnf = CardEnc.atleast(lhs, bound=kval + dval, top_id=self.nv, encoding=enc)
        self.clauses.extend(cnf.clauses)
        self.nv = cnf.nv


#
#==============================================================================
def parse_options():
    """
        Parses command-line options:
    """

    try:
        opts, args = getopt.getopt(sys.argv[1:], 'd:e:hk:m:',
                ['dval=', 'enc=', 'help', 'kval=', 'mval='])
    except getopt.GetoptError as err:
        sys.stderr.write(str(err).capitalize())
        usage()
        sys.exit(1)

    enc = 'kmtot'
    dval = 1
    kval = 1
    mval = 8

    for opt, arg in opts:
        if opt in ('-d', '--dval'):
            dval = int(arg)
        elif opt in ('-e', '--enc'):
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
    return enc, dval, kval, mval


#
#==============================================================================
def usage():
    """
        Prints usage message.
    """

    print('Usage:', os.path.basename(sys.argv[0]), '[options]')
    print('Options:')
    print('        -d, --dval=<int>      Difference between two constraints (d)')
    print('                              available values: [1 .. int_max] (default = 1)')
    print('        -e, --enc=<string>    Cardinality encoding')
    print('                              Available values: cardn, kmtot, mtot, seqc, sortn, tot (default = kmtot)')
    print('        -h, --help')
    print('        -k, --kval=<int>      right-hand side (k)')
    print('                              available values: [1 .. int_max] (default = 1)')
    print('        -m, --mval=<int>      Size of the left-hand size (m)')
    print('                              Available values: [0 .. INT_MAX] (default = 8)')

#
#==============================================================================
if __name__ == '__main__':
    enc, dval, kval, mval = parse_options()

    assert mval > kval + dval, 'Wrong parameters are chosen!'

    cnf = TwoCC(mval, kval, dval, enc)
    cnf.to_fp(sys.stdout)
