#!/bin/bash
script_name="run_pimue.C"
input_pattern="pimue"
echo "Running custom script"
echo "LD_LIBRARY_PATH is ${LD_LIBRARY_PATH}"
source /simulation/set_env.sh
echo "ls -lst /simulation/install/lib"
ls -lst /simulation/install/lib

infile=$1
if [[ "$infile" = "." ]]; then
    ls ${input_pattern}*.root > inputfile.txt
    infile="inputfile.txt"
    trashfiles=`cat $infile`
    echo "Inputfile is ${infile}"
    for i in ${files[@]}; do
        echo "----> $i"
    done
else
    echo "Inputfile is ${infile}"
fi

ls -lst
root -b -q "${script_name}(\"${infile}\")"

#clean 
echo "Cleaning input file"
rm -f $trashfiles
rm $infile
