#!/bin/bash
script_name="run_pienu.C"
echo "Running custom script"
echo "LD_LIBRARY_PATH is ${LD_LIBRARY_PATH}"
source /simulation/set_env.sh
echo "ls -lst /simulation/install/lib"
ls -lst /simulation/install/lib
infile=$1
echo "Inputfile is ${infile}"
ls -lst
root -b -q "${script_name}(\"${infile}\")"
#clean 
echo "Cleaning input file"
rm $infile
