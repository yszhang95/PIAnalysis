# PIAnalysis
A repository for analyzing simulated data.
## HOWTO
### Download
Replace the HTTPS link with your own folk. I prefer to use ssh link instead.
```shell
git clone --recurse-submodules https://github.com/yszhang95/PIAnalysis.git
```

### Build
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
  perl -i.bak -pe 's/Get(.+?)\(\)/Get$1() const/g' PIMCAtar.hh
  cmake -B build -S . \
  -DROOT_DIR=/opt/local/libexec/root6/share/root/cmake \
  -DCMAKE_INSTALL_PREFIX=install -DCMAKE_EXPORT_COMPILE_COMMANDS=1
  cmake --build build --target install
```

### Run macro
Edit the input in `acceptance/test/run_hit_merger.C` and validate it.
Then do
```shell
cd acceptance/test
# root -b -q run_hit_merger.C
# prepare a small ntuple of fast simulated hits using PIAnalyzer
root -b -q run_test.C
# event display
# assume the input file exists. Check the input file in run_hit_merger.C
./display_track.sh 12
```
