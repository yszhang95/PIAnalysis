#!/bin/bash
ContainerPathBak="/data/eliza5/PIONEER/containers/pioneer_latest.sif"
FrameworkPathBak="/home/yousen/production-Mar-2024/main"
ContainerPath=$1
FrameworkPath=$2
TopDir=$(git rev-parse --show-toplevel)

if [ "${ContainerPath}" = "" ]; then
    ContainerPath=${ContainerPathBak}
fi
echo "Setting Container Path to ${ContainerPath}"

if [ "${FrameworkPath}" = "" ]; then
    FrameworkPath=${FrameworkPathBak}
fi
echo "Setting Framework Path to ${FrameworkPath}"
echo

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
BuildDir=/tmp/PIAnalysis_build
mkdir /tmp/PIAnalysis_build && cd /tmp/PIAnalysis_build
cmake /tmp/PIAnalysis_src \
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

echo "Source directory is $TopDir"
singularity exec -B /tmp:/tmp -B ${TopDir}:/tmp/PIAnalysis_src -B $FrameworkPath:/simulation $ContainerPath bash tmp.sh

rm tmp.sh
