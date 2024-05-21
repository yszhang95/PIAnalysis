#!/bin/bash

#$ -j y 					#merge stderr and stdout
#$ -m n 					#don't send an email about this job
#$ -S /bin/bash             #use bash shell
#$ -l scratch=25G           #ensure we have enough disk space for output files.
#$ -l mem_req=6.0G          #Make sure there is enough ram for program to run

ROOTSTORAGE="/state/partition1" #directory where we can store static files, like root/geant installations
# ROOTSTORAGE=$TMPDIR #Change to allow multiple people to run the script

echo "Starting grid job"
echo "   -> Job ID: " $JOB_ID
echo "   -> Task ID: " $SGE_TASK_ID

nArgs=$(echo $#)
echo "Number of arguments:" $nArgs
echo $@

scriptPath=$1             # the path to the compiled simulation DEPRECATED
script=$2                 # the name of the compiled simulation DEPRECATED
infilepath=$3             # the path to an input file, here the singularity container (.sif)
infile=$4                 # the name of an input file, here the singularity container (.sif)
outdirbase=$5             # the name of the directory where we should direct output
nevents=$6                # the number of events per job to run DEPRECATED
macPath=$7                # the path to the .mac file
macfile=$8                # the name of the .mac file
compileTar=$9             # the tar file to be extracted/compiled
compileFolder=${10}       # the top level folder of the tar file to be extracted/compiled
geometryPath=${11}        # path to the .gdml geometry file
geometry=${12}            # name of the .gdml geometry file
nCores=${13}              # the number of cores to use in this simulation 
postPath=${14}              # the path to the post-processing script
post=${15}                  # the name of the post-processing script
py_use_directory=${16}    # whether to use the directory or filenames when passing root files to post-process DEPRECATED
delete_root_file=${17}    # whether to delete the root files before copying back to output directory
required_file_path=${18}    # path to the required files directory, copy everything from this dir.
setup_options=${19} #options to pass the setup script, default -ra
input_root_file_path=${20}    # path to the input root file directory, copy the file matching the SGE task id from this directory
do_simulation=${21}    # whether to run G4Pioneer or whether to just proceed with running the post-processing (i.e. if we're reprocessing already make simulation files)

queue_dir="/data/eliza5/PIONEER/queue/" # hack for a queue system, make sure we don't have 500 read writes at the same time
datestring=$(date +%Y_%m_%d_%H_%M_%S_%N)
echo "Date string: " $datestring
outdir=${outdirbase}/${datestring}
if [ -d "${outdir}" ]
then
	echo "Directory $outdir exists."
	outdir="${outdir}_1"
	echo "New outdir: $outdir"
fi
logfile="log_"${script}"_"$datestring".log"
postlogfile="log_"${script}"_"$datestring"_post_process.log"

echo $scriptPath $script $infilepath $infile $outdir

#make sure the output directory exists
mkdir -p $outdir

date
echo $PATH

#copy over the fcl file and the geant4 tarball (if it doesn't exist already)
echo "**********************************************************************"
cd $TMPDIR
echo "Temporary directory: "
pwd -P

ls -ltrh $TMPDIR

echo "**********************************************************************"
echo "Macro directory:"
ls -ltrh ${macPath}

echo "**********************************************************************"
echo "Macro file contents:"
cat ${macPath}/${macfile} | head -10
echo "..."
cat ${macPath}/${macfile} | tail -10

echo "**********************************************************************"
echo "Copying files..."
t1=`date +%s`
container=$ROOTSTORAGE/pioneer_container_${datestring}.sif
echo "Creating unique container for this job: ${container}"
while :; do
	nfiles=$(ls -l ${queue_dir} | wc -l)
	echo "$nfiles in directory"

	if [ "$nfiles" -le "5" ]; then
		echo "     -> Processing!"
		touch ${queue_dir}${datestring}
		# if [[ ! -f $ROOTSTORAGE/${infile} ]]
		# then
		# echo "${infile} does not exist on this filesystem. copying now."
		# if [[ -f "${ROOTSTORAGE}/${infile}" ]]; then
		# 	echo "${infilepath}/${infile} found! No need to copy."
		# else
		time rsync -tvz --copy-links --progress ${infilepath}/${infile} ${container}
		# fi
		# fi
		# ${postPath}/${post}  \
		# ${scriptPath}/${script} \
		these_input_files="input_files_${SGE_TASK_ID}.txt"
		time rsync -vtz --copy-links --progress \
			${macPath}/${macfile} \
			${geometryPath}/${geometry} \
			${postPath}/${post} \
			${required_file_path}/* \
			${input_root_file_path}/${these_input_files} \
			${compileTar} $TMPDIR/

		if [ -f "${these_input_files}" ]; then
			echo "$these_input_files exists -> copying files"
			time rsync -avh --progress --copy-links \
				`cat ${these_input_files}` \
				${TMPDIR}/
		fi

		rm -f ${queue_dir}${datestring}
		break
	fi
	echo "     queue is full -> waiting..."
	sleep 10
	# waiting=
done
t2=`date +%s`
echo "Done!"
echo "Waited for $(($t2-$t1)) seconds"

pwd
ls -ltrh
ls -ltrh ${ROOTSTORAGE}


cd ${TMPDIR}

#untar the compilation folder
echo "Untarring ${compileTar}"
tar -zxf ${compileTar}
echo "Contents of TMPDIR:"
pwd
ls -ltrh

echo "Contents of compile folder (${compileFolder}):"
ls -ltrh ${compileFolder}

output_file=${TMPDIR}/"output_"${mac_file}".root"
process="1"

# change the random seeds in the macro file
for J in $(seq 1 5); do
	echo $RANDOM >>tmp.txt
done
NUMBERS=$(paste -sd' ' <tmp.txt)
rm tmp.txt
sed "s/setSeeds/setSeeds $NUMBERS/" <$macfile >"${macfile}.seeded"
echo "${macfile}.seeded"
macfile="${macfile}.seeded"

echo "Seeds: "
cat $macfile | grep 'Seed'


cd $TMPDIR
#produce the script that will be run within the singularity container
singularity_script="singularity.sh"
cat > $singularity_script <<EOF

echo "Beginning work in container..."

source /software/setup_container_env.sh
cd /simulation/
./setup.sh -${setup_options}

echo "***************** /simulation ****************"
ls -ltrh
echo "***************** /simulation ****************"

cd /output
echo "***************** /output ****************"
ls -ltrh
echo "***************** /output ****************"

if [[ "${do_simulation}" -eq "1" ]]; then
	if [ "$nCores" -eq "-1" ]; then
		# use all available cores on this compute node, done by default
		echo "Running simulation!"
	else
		# use only the specified number of cores.
		export G4FORCENUMBEROFTHREADS=${nCores}
		echo "Running simulation (with \${G4FORCENUMBEROFTHREADS} cores)!"
	fi

	time G4Pioneer ${macfile} > $logfile 2>&1
	echo "Simulation complete. See log file: $logfile"
else
	echo "Simulation skipped. Running post-processing directly on input files."
fi
chmod 777 ./*root

echo "***************** /output (post-run) ****************"
ls -ltrh
echo "***************** /output (post-run) ****************"


echo "***************** Running gaudi/python post process *********************"

files_to_process="\`ls ./*root\`"
echo "Files to process:" \$files_to_process

if [[ "${py_use_directory}" -eq "1" ]]; then
	echo "Passing directory: ."
	source ${post} . | tee ${postlogfile}
else
	for FILE in \$files_to_process; do 
		source ${post} \$FILE | tee \${FILE}_${postlogfile}
	done
fi


echo "******************* Results ***********************"
ls -ltrh

echo "All done in container!"

EOF

echo "Singularity script:"
echo "******************************************************"
cat $singularity_script
echo "******************************************************"
chmod 777 $singularity_script

#Mount syntax ->  -B /path/to/external:/path/within/container
time singularity exec \
	-B $TMPDIR/$compileFolder:/simulation \
	-B $TMPDIR:/output \
	-B /tmp:/tmp  \
	--containall ${container} bash -c "/output/$singularity_script"

ls -ltrh


if [[ "${delete_root_file}" -eq "1" ]]; then
	echo "Deleting the root output from the simulation."
	rm -f ./*root
	ls -ltrh
else
	echo "Returning root files and pickled post-process."
fi

#copy the output file back to the correct directory
while :; do
	nfiles=$(ls -l ${queue_dir} | wc -l)
	echo "$nfiles in directory"

	if [ "$nfiles" -le "5" ]; then
		echo Processing!
		touch ${queue_dir}${datestring}
		rsync -vz --progress  \
			$TMPDIR/*root \
			$TMPDIR/*pickle \
			$TMPDIR/*.png \
			$TMPDIR/*pienux_out* \
			$TMPDIR/*log \
			$TMPDIR/*.mac* \
			$TMPDIR/*.sh \
			$FILE ${outdir}/
		rm -f ${queue_dir}${datestring}
		break
	fi
	echo "waiting..."
	sleep 10
done

echo "Removing container..."
rm -f ${container}

echo ""
echo "******************************************************"
echo "******************************************************"
echo "******************************************************"
echo "All done!"
echo "******************************************************"
echo "******************************************************"
echo "******************************************************"

