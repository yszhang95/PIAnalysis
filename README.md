# PIAnalysis
A repository for analyzing simulated data.
## HOWTO
### From docker
#### Download
Download necessary part without submodules to original central shared libraries.
```shell
git clone https://github.com/yszhang95/PIAnalysis.git
```
#### Build
I assume we are under docker environment.
To work under docker environment,
```shell
git clone --recursive git@github.com:PIONEER-Experiment/main.git -b develop
cd main
./checkout-branches.sh
cd shared/include
perl -i.bak -pe 's/Get(.+?)\(\)/Get$1() const/g if ! /static/' PIMCAtar.hh
perl -i.bak -pe 's/Get(.+?)\(\)/Get$1() const/g if ! /static/' PIMCInfo.hh
cd ../../../
podman run -v ${PWD}/main:/simulation -v /tmp:/tmp --hostname=PIONEER --name=PIONEER --rm -it docker://jlabounty/pioneer:latest
```
Inside docker, do
```shell
# new env
. /software/setup_container_env.sh
# compile central framework
cd /simulation
. /simulation/setenv.sh
/simulation/setup.sh -be

cd /tmp
git clone https://github.com/yszhang95/PIAnalysis.git /tmp/PIAnalysis_src
mkdir /tmp/PIAnalysis_build && cd /tmp/PIAnalysis_build
cmake /tmp/PIAnalysis_src \
  -DCMAKE_INSTALL_PREFIX=${PIONEERSYS}/install \
  -DUSE_PIONEER_DOCKER=1
make
make install
```
### From scratch
#### Download
Replace the HTTPS link with your own folk. I prefer to use ssh link instead.
```shell
git clone --recurse-submodules https://github.com/yszhang95/PIAnalysis.git
```

#### Build
- Some customization are necessary for reading data, replace
  `CMakeLists.txt` under `shared` with `shared_cmake.txt`.
- Update the content in `shared/include/PIMCAtar.hh`. Convert getters
  to constant version
- Configure `cmake` and build. Replace
  `/opt/local/libexec/root6/share/root/cmake` with the path it
  actually is on one's own OS.
An example is
```shell
  cp shared_cmake.txt shared/CMakeLists.txt
  cd shared/include
  perl -i.bak -pe 's/Get(.+?)\(\)/Get$1() const/g if ! /static/' PIMCAtar.hh
  perl -i.bak -pe 's/Get(.+?)\(\)/Get$1() const/g if ! /static/' PIMCInfo.hh
  mkdir ../PIAnalysis_build && cd ../PIAnalysis_build
  # replace ROOT_DIR with your own setup
  cmake ../PIAnalysis \
  -DROOT_DIR=/opt/local/libexec/root6/share/root/cmake \
  -DCMAKE_INSTALL_PREFIX=${PWD}/../PIAnalysis_install -DCMAKE_EXPORT_COMPILE_COMMANDS=1
  cmake --build build --target install
```

#### Run macro
Edit the input in `acceptance/test/test_job.C` and validate it.
Then do
```shell
cd acceptance/test
root -b -q test_job.C
```

#### Generate a set of user-defined actions
```shell
# assume we are on the top of git repository
./tools/mk_piaction.py --dir acceptance --type analyzer PITestAnalyzer
# acceptance/include/PITestAnalyzer.hpp generated
# acceptance/src/PITestAnalyzer.cpp generated
```
