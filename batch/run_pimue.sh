#!/bin/bash
infile=$(sed -n "${1},${1}p" pimue_filelists.txt)
infile=$(basename $infile)
echo "Input file is $infile"
ls -lst
tar zxf PIAnalysis.tar.gz
mv PIAnalysis /tmp/PIAnalysis_src
ls -lst /tmp
root -b -q "run_pimue.C(\"${infile}\")"
