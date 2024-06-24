#!/usr/bin/env python3
import hashlib
import subprocess
import os
import argparse
import shutil

import time

import collections

parser = argparse.ArgumentParser(
        prog='merge_histograms.py',
        description='merge histograms in ROOT',
        epilog='Only use it when you know what you are doing.')

parser.add_argument('-i', dest='inpath', type=str, nargs='+',
        required=True, help='path to input file')
parser.add_argument('-o', dest='ofile', type=str,
        required=True, help='name of output file')

args = parser.parse_args()
print(args.ofile)

tempdir="/tmp/{}".format(int(time.time()))
os.mkdir(tempdir)
print("Prepared temporary directory {}".format(tempdir))


nmax = 150

ofs = []
temps = []
for i, f in enumerate(args.inpath):
    of = "{}/{}.root".format(tempdir, hashlib.md5(f.encode('utf-8')).hexdigest())
    if of not in ofs:
        ofs.append(of)
        subprocess.run(["rootcp", "{}:h_*".format(f), of])
    else:
        raise ValueError("Duplicate temporary files. Refine the algorithm")
        exit(-1)
    # temporary merge
    if i % nmax == 0 and i != 0:
        obn = os.path.basename(args.ofile)
        output = "{}/{}.{}".format(tempdir, obn, int(time.time()))
        temp_cmd = ["hadd", "-f", output] + ofs

        # save
        temps.append(output)
        # merge
        subprocess.run(temp_cmd)
        # log
        print("=========== merged {} ===========".format(nmax))
        # clean
        ofs = []
if len(ofs):
    obn = os.path.basename(args.ofile)
    output = "{}/{}.{}".format(tempdir, obn, int(time.time()))
    temp_cmd = ["hadd", "-f", output] + ofs
    # save
    temps.append(output)
    # merge
    subprocess.run(temp_cmd)
    print("========== merged {} ==========".format(len(ofs)))
    # clean
    ofs = []

# hadd
cmds = ["hadd", "-f", args.ofile] + temps
subprocess.run(cmds)

# clean
shutil.rmtree(tempdir)
