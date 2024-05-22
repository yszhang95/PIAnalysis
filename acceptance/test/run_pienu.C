#include <Rtypes.h>
#ifdef __CLING__
R__LOAD_LIBRARY(../../../PIAnalysis_install/lib/libPiAnaAcc.so)
R__LOAD_LIBRARY(../../../PIAnalysis_install/lib/libPiRootDict.so)
#else
#include "PITrueDecPos.hpp"
#include "PIEvtNbFilter.hpp"
#include "PIAnaConst.hpp"
#include "PIFilterPiDAR.hpp"
#include "PIHitProducer.hpp"
#include "PITopoProducer.hpp"
#include "PITrueMomProducer.hpp"
#include "PIJobManager.hpp"
#include "PITreeAnalyzer.hpp"
#include "PIAccAnalyzer.hpp"
#endif

void run_pienu()
{

    gErrorIgnoreLevel = kError;

  PIAna::PIJobManager pi;
  pi.treename("sim");

  std::string filename;
  ifstream filelists("pienu_files.txt");
  while (std::getline(filelists, filename)) {

      if (filename[0] != '#') {

      pi.add_file(filename);
      std::cout << "Added "<< filename << "\n";
      }
  }

  //pi.out_file("pienu_output_20240430.root");
  pi.out_file("pienu-output-test.root");
  pi.add_action("PiDAR",
                std::make_unique<PIAna::PIPiDARFilter>("PiDAR", static_cast<int>(PIAna::EvtCode::PiDAR)));
  pi.add_action("DecPos", std::make_unique<PIAna::PITrueDecPos>("DecPos"));
  auto decpos = dynamic_cast<PIAna::PITrueDecPos *>(pi.get_action("DecPos"));
  pi.add_action("TrueEMom", std::make_unique<PIAna::PITrueMomProducer>("TrueEMom"));
  auto emom = dynamic_cast<PIAna::PITrueMomProducer *>(pi.get_action("TrueEMom"));
  emom->decay_mode(PIAna::PITrueMomProducer::PiDecayMode::pienu);
  pi.add_action("RecHits", std::make_unique<PIAna::PIHitProducer>("RecHits"));
  auto rechits = dynamic_cast<PIAna::PIHitProducer *>(pi.get_action("RecHits"));
  rechits->de_thres(0.0155);
  pi.add_action("Topo", std::make_unique<PIAna::PITopoProducer>("Topo"));
  auto topoproducer =
      dynamic_cast<PIAna::PITopoProducer *>(pi.get_action("Topo"));
  topoproducer->min_tcluster(2);
  pi.add_action("AccAna", std::make_unique<PIAna::PIAccAnalyzer>("AccAna"));
  auto accana = dynamic_cast<PIAna::PIAccAnalyzer *>(pi.get_action("AccAna"));
  accana->topoflag_name("Topo_flag");
  accana->pivertex_name("Topo_pivertex");
  accana->estart_name("Topo_estart");
  accana->ehits_name("Topo_ehits");
  accana->edirection_name("Topo_edirection");
  accana->true_pivertex_name("DecPos_pivertex");
  accana->true_estart_name("DecPos_estart");
  accana->etruemom_name("TrueEMom_emom");
  pi.begin();
  pi.run();
  pi.end();
}
