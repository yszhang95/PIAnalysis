#!/bin/bash
usage() { echo "Usage: $0 [-c <container-path>] [-p <framework-path>] [-t <commit-or-branch>] [-f]" 1>&2; exit 0; }

while getopts "hc::p::t::f" opt; do
    case "$opt" in
        h)
            usage
            ;;
        c)
            ContainerPath=${OPTARG}
            ;;
        p)
            FrameworkPath=${OPTARG}
            ;;
        t)
            CommitPoint=${OPTARG}
            ;;
        f) 
            force=1
            ;;
    esac
done

shift $((OPTIND-1))

ContainerPathBak="/data/eliza5/PIONEER/containers/pioneer_latest.sif"
FrameworkPathBak="/home/yousen/production-Mar-2024/main"

if [ "${ContainerPath}" = "" ]; then
    ContainerPath=${ContainerPathBak}
fi
echo "Setting Container Path to ${ContainerPath}"

if [ "${FrameworkPath}" = "" ]; then
    FrameworkPath=${FrameworkPathBak}
fi
echo "Setting Framework Path to ${FrameworkPath}"
echo

if [ "${CommitPoint}" = "" ]; then
    CommitPoint="main"
fi

# prepare tar
OldDir=$(pwd)
GitTopDir=$(git rev-parse --show-toplevel)
# https://unix.stackexchange.com/a/230676
RndStr=$(tr -dc A-Za-z0-9 </dev/urandom | head -c 13; echo)
TopDir="/tmp/PIAnalysisTemp${RndStr}/"
PIAnaSrcDir="${TopDir}"
TarStr="PIAnalysis${RndStr}.tar"
mkdir $PIAnaSrcDir
# https://stackoverflow.com/a/27452248
#git -C "$(git rev-parse --show-toplevel)" archive --format=tar -o ${TarStr} --prefix=${PIAnaSrcDir} ${CommitPoint}
cd $GitTopDir
if [[ "$force" -eq 1 ]]; then
    echo "To tar files under path ${PWD}"
    tar -cvf ${TarStr} --exclude-vcs \
        --exclude="*.root" --exclude="*.o" --exclude="*.so" \
        --exclude="*.rootmap" --exclude="*.pcm" \
        --exclude="*__pycache__" --exclude="*venv*" \
        --exclude="PIAnalysis*.tar" \
        .
else
    echo "To tar files under commit/branch ${CommitPoint}"
    git archive -v --format=tar -o ${TarStr} ${CommitPoint}
fi
mv $TarStr ${PIAnaSrcDir} && cd ${PIAnaSrcDir} && tar xf ${PIAnaSrcDir}/${TarStr}
cd ${OldDir}
echo

# prepare compiling script
DockerPIAnaSrc="/tmp/PIAnalysis_src"
DockerPIAnaSrcTarget="/simulation/PIAnalysis_src"

cat > tmp.sh << EOF
echo "Setting up environment variables"
. /software/setup_container_env.sh
echo

echo "Compiling central framework"
cd /simulation
. /simulation/setenv.sh
/simulation/setup.sh -be
echo

echo "Compiling PIAnalysis"


rm -rf ${DockerPIAnaSrcTarget} && mkdir -p ${DockerPIAnaSrcTarget}
mv ${DockerPIAnaSrc}/* ${DockerPIAnaSrcTarget}
ls -lst ${DockerPIAnaSrcTarget}

BuildDir=/tmp/PIAnalysis_build
mkdir /tmp/PIAnalysis_build && cd /tmp/PIAnalysis_build
cmake ${DockerPIAnaSrcTarget} \
  -DCMAKE_INSTALL_PREFIX=\${PIONEERSYS}/install \
  -DUSE_PIONEER_DOCKER=1
make
make install
echo

# clean
cd /
rm -r \${BuildDir}
echo "Done and exit"
EOF

echo "Source directory is $PIAnaSrcDir"
singularity exec -B /tmp:/tmp -B ${PIAnaSrcDir}:${DockerPIAnaSrc} -B $FrameworkPath:/simulation $ContainerPath bash tmp.sh

rm tmp.sh
