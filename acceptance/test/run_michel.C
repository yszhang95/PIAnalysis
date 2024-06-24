#include <Rtypes.h>
#ifdef __CLING__
// run inside docker
R__LOAD_LIBRARY(libPiAnaAcc.so)
R__LOAD_LIBRARY(libPiRootDict.so)
#else
#include "PIMichelEnergyAnalyzer.hpp"
#include "PIJobManager.hpp"
#endif

void run_michel()
{
  // gErrorIgnoreLevel = kError;

  PIAna::PIJobManager pi;
  pi.treename("sim");

  std::string filename;
  ifstream filelists("pimue_files.txt");
  while (std::getline(filelists, filename)) {
    if (filename[0] != '#') {
      pi.add_file(filename);
      std::cout << "Added "<< filename << "\n";
    }
  }

  pi.out_file("michel_energy_spectrum.root");
  pi.add_action("MichelEnergy", std::make_unique<PIAna::PIMichelEnergyAnalyzer>("MichelEnergy"));
  auto michel_energy = dynamic_cast<PIAna::PIMichelEnergyAnalyzer *>(pi.get_action("MichelEnergy"));
  pi.begin();
  pi.run();
  pi.end();
}
