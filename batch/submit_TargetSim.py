'''
Utility to parse parameteters and send a qsub command with the relevent information to the CENPA Rocks 
compute nodes. Utilizes container defined in singularity.def
Requires external shell script (run_geant_container.sh).

Tested on Python 3.8-3.9. REQUIRES Python 3!

Example Usage:
        python submit_TargetSim.py --help
    	python submit_TargetSim.py \
            --infile /home/labounty/github/MonteCarlo/build/G4Pioneer  \
            --geometry /home/labounty/github/MonteCarlo/geometry/pienux.gdml \
            --mac /home/labounty/github/MonteCarlo/build/macros/beam.mac \
            --outpath /home/labounty/data/users/labounty/NewSimTest \
            --container  /home/labounty/pienux.sif \
            --compile-path /home/labounty/github/MonteCarlo/build \
            --njobs 2 \
            --nevents 500 \
            --ncores 1 
'''


import numpy as np
import os
import sys

import argparse
import datetime
import time
import random
import subprocess

# https://stackoverflow.com/a/43357954
def str2bool(v):
    if isinstance(v, bool):
        return v
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0'):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')

def copy_file_to_outpath(infile, path, istest):
    '''
        Function to copy a file to a new directory and return the path to the directory containing the file
    '''
    if not (os.path.exists(infile)):
        raise FileNotFoundError("ERROR: File to copy not found ->"+infile)

    print("Copying", os.path.basename(infile), "to new directory")
    command = f'rsync -ah --copy-links --progress {infile} {path}'
    if(not istest):
        os.system(command)
    return path


def main():

    if(sys.version_info[0] < 3):
        raise ValueError("ERROR: You must use python 3 to run this script (preferably 3.8+)")

    jobstring = " "

    parser = argparse.ArgumentParser(
        description='Process grid submission options')
    parser.add_argument("--infile", required=False, type=str,
                        help='Path to the compiled simulation code ')
    parser.add_argument("--outpath", required=True, type=str,
                        help='Where to copy output files to')
    parser.add_argument("--container", required=True, type=str, 
        help="Path to the singularity container in which root/geant are installed.")
    parser.add_argument("--compile-path", required=True, type=str,
                        help='Path to be tarred up and transferred to the grid, containing the simulation code (i.e. the main directory)')
    parser.add_argument("--njobs", default=1, type=int,
                        help='Number of unique jobs to submit')
    parser.add_argument("--nevents", default=1, type=int,
                        help='Number of events each job will process')
    parser.add_argument("--cores", '--ncores', default=-1, required=False, type=int,
                        help="Number of cores to use on each grid node. If -1, will use maximum available.")
    parser.add_argument("--macfile", '--macro', '--mac', required=False, default=None,
                        help='Existing mac file to run, overrides nevents and other physics settings')
    parser.add_argument("--geometry", required=True,
                        type=str, help="Path to the geometry file")
    parser.add_argument("--copy-files", required=False, type=bool, default=True,
                        help="Will files necessary for the script be copied into the output directory before running")
    parser.add_argument("--test", '--dry-run', required=False, type=bool, default=False,
                        help="If this option is set, will not run any commands. Will simply output to the console")
    parser.add_argument("--additional-args", default='', nargs='+')
    parser.add_argument("--full-output", type=bool, default=False)
    parser.add_argument("--post-process-script", type=str, default='', 
                        help='Path to a bash script to be run on the output root file')
    parser.add_argument("--pass-directory-to-script", type=str2bool, default=True)
    parser.add_argument("--delete-root-files", help='Remove the root files after post-processing', action='store_true')
    parser.add_argument("--required-files", default='', nargs='+')
    parser.add_argument("--setup-options", required=False, type=str, default='ra')
    parser.add_argument("--input-files", required=False, type=str, default=None)
    parser.add_argument("--exclude-root", required=False, type=bool, default=False)
    parser.add_argument("--do-simulation", required=False, type=str2bool, default=True)

    ding = parser.parse_args(sys.argv[1:])

    if(".sif" not in ding.container):
        raise RuntimeWarning("Warning: container suffix not detected. Are you sending in a singularity container?")
    if(not os.path.exists(ding.container.strip())):
        raise FileNotFoundError(f"ERROR: Unable to locate singularity container: {ding.container}")

    # parse the output location
    # and create a subfolder so all outputs/logs are collated
    ding.outpath = os.path.abspath(ding.outpath)
    new_output = ding.outpath.rstrip(
        "/")+"/job_submission_"+str(int(time.time()))
    ding.outpath = new_output
    # if(not ding.test):
    os.system('mkdir -p '+ding.outpath)
    print("Storing output files in:", ding.outpath)

    # parse the path to the input simulation file, DEPRECATED
    jobstring += 'NULL' + " " + 'NULL' + " "

    # parse the full simulation tarball location
    container_path, container_file = os.path.split(ding.container)
    jobstring += container_path + " " + container_file + " "

    # add the output file location to the jobstring
    jobstring += ding.outpath + ' '

    # parse the number of muons
    if(ding.nevents < 1):
        raise ValueError("ERROR: Must have more than one event per job")
    jobstring += str(ding.nevents) + " "

    # process the .mac file
    mac_file = "nomac"
    if (ding.macfile is None):
        # if not ding.do_simulation: # if we're only doing post-processing, we don't need the mac file
        #     raise NotImplementedError
        if ding.do_simulation:
            raise ValueError("ERROR: --do-simulation must be false when no mac file is given")
        jobstring += f' NULL NULL '
    else:
        mac_file = os.path.basename(ding.macfile)
        if(ding.copy_files):
            mac_path = copy_file_to_outpath(ding.macfile, ding.outpath, ding.test)
        else:
            mac_path = os.path.dirname(ding.macfile)
        jobstring += mac_path + " " + mac_file + " "

    # tar up and add the directory to be compiled to the jobstring
    tar_file_compile = os.path.join(ding.outpath,'pietar.tar.gz')

    ding.compile_path = ding.compile_path.rstrip("/")
    print("Compile path:", ding.compile_path)
    assert os.path.exists(ding.compile_path)
    tar_path_2, tar_dir = os.path.split(ding.compile_path)
    tarcom = f'tar {"--exclude=*.root" if ding.exclude_root else ""} --exclude=*.sif --exclude=*.pickle --exclude=*.gdml -zcf ' + \
        str(tar_file_compile)+' -C ' + \
        tar_path_2+' '+tar_dir
    print(tarcom)
    if(not ding.test):
        os.system(tarcom)
    jobstring += tar_file_compile + ' ' + \
        tar_dir + ' '

    # specify the geometry file
    if(not os.path.exists(ding.geometry)):
        raise FileNotFoundError(
            "ERROR: Unable to open geometry file:"+ding.geometry)
    if(ding.copy_files):
        geometry_path = copy_file_to_outpath(
            ding.geometry, ding.outpath, ding.test)
    else:
        geometry_path = os.path.dirname(ding.geometry)
    geometry_file = os.path.basename(ding.geometry)
    jobstring += " "+geometry_path+' '+str(geometry_file)+' '

    # specify the number of cores on each node
    if(ding.cores < 1 and ding.cores != -1):
        raise ValueError("Number of cores must be > 1 or -1 (all)")
    jobstring += " "+str(ding.cores)+' '

    # specify a python script to run on the .root output after geant has finished.
    if(len(ding.post_process_script) > 1):
        if(not os.path.exists(ding.post_process_script)):
            raise FileNotFoundError("ERROR: Unable to open post process script file:"+ding.post_process_script)
        if('.sh' not in ding.post_process_script[-5:]):
            raise ValueError("Must pass in a bash script (ending in .sh)")
        if(ding.copy_files):
            py_path = copy_file_to_outpath(ding.post_process_script, ding.outpath, ding.test)
        else:
            py_path = os.path.dirname(ding.post_process_script)

        py_file = os.path.basename(ding.post_process_script)
        jobstring += " "+py_path+' '+str(py_file)+' '
    else:
        # to simplify the bash script, create a dummy python script if we pass in no post-processing
        dummy_py = os.path.join(ding.outpath, "temp.sh")
        with open(dummy_py, 'w') as fout:
            fout.write("echo No work to do in post-processing! \n")
        jobstring += " "+ding.outpath+' '+os.path.basename(dummy_py)+' '

    # whether to pass directory or the individual files to the post-process-script
    jobstring += f' {int(ding.pass_directory_to_script)} {int(ding.delete_root_files)} '

    # copy over required files (input to post-processing script)
    reqs = ding.required_files
    req_path = os.path.join(ding.outpath, 'required_files')
    os.system(f'mkdir -p {req_path}')
    if len(reqs) > 0:
        for fi in reqs:
            copy_file_to_outpath(
                fi, req_path, ding.test
            )
    jobstring += f' {req_path} '

    # pass along the setup script options
    jobstring += f' {ding.setup_options} '

    # parse the input filelist
    input_file_path = os.path.join(ding.outpath,'input_files')
    os.system(f'mkdir -p {input_file_path}')
    if ding.input_files is not None:
        assert os.path.exists(ding.input_files)
        # ding.njobs
        copy_file_to_outpath(ding.input_files, input_file_path, ding.test)
        with open(ding.input_files, 'r') as fin:
            files = [x.strip() for x in fin]
        lists = np.array_split(files, ding.njobs)
        for i, li in enumerate(lists):
            fi = os.path.join(input_file_path, f'input_files_{i+1}.txt') #SGE task ID starts at 1
            with open(fi, 'w') as fout:
                for lii in li:
                    assert os.path.exists(lii)
                    fout.write(f'{os.path.abspath(lii)}\n')
    jobstring += input_file_path 

    # parse whether to do the simulation at all
    jobstring += f" {int(ding.do_simulation)} "

    # create a log file with relevent information about the submission
    # git_commit = subprocess.check_output(
    #     [f"cd {os.path.dirname(ding.infile)};", "git", "describe", '--always'], shell=True).strip()
    # git_commit = subprocess.run(f"cd {os.path.dirname(ding.infile)}; git describe --always", shell=True, capture_output=True).stdout
    # print("Current commit version:", git_commit)
    submission_log = os.path.join(ding.outpath, 'submission.log')
    with open(submission_log, 'w') as logfile:
        # logfile.write(f"Current git commit: {git_commit}\n")
        logfile.write(f"Input: {sys.argv} \n")
        logfile.write(f"Parsed: {ding} \n")
        logfile.write(f"Jobstring: {jobstring} \n")

    # finally, add qsub parameters such a name, log directories, etc.
    name = mac_file
    logdir = ding.outpath.rstrip("/")+'/logs/'
    if (not ding.test):
        os.system("mkdir -p "+logdir)
    jobstring = f'qsub {f"-t 1-{ding.njobs}" if ding.njobs > 1 else ""} -N '+name+' -e '+logdir + \
        ' -o '+logdir+' run_geant_container.sh '+jobstring

    # parse the number of jobs, and run qsub for each one
    # for _ in range(ding.njobs):
    print(jobstring)
    #     # break
    # input("so?")
    if(not ding.test):
        os.system(jobstring)
        # time.sleep(0.2) #just to ensure the folders have different names
    print("Log files written to:", logdir)
    print("Output files written to:", ding.outpath)


if __name__ == "__main__":
    main()
