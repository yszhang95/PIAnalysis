#ifdef __CLING__
R__LOAD_LIBRARY(libPiAnaAcc.so)
R__LOAD_LIBRARY(libPiRootDict.so)
#else
#include <Rtypes.h>
#include "PITrueDecPos.hpp"
#include "PIEvtNbFilter.hpp"
#include "PIAnaConst.hpp"
#include "PIFilterPiDAR.hpp"
#include "PIHitProducer.hpp"
#include "PITopoProducer.hpp"
#include "PIJobManager.hpp"
#include "PITreeAnalyzer.hpp"
#include "PIAccAnalyzer.hpp"
#endif

void run_pienu()
{
  PIAna::PIJobManager pi;
  pi.treename("sim");
  pi.add_file("pienu-test.root");
  pi.out_file("pienu-job-output.root");
  pi.add_action("PiDAR",
                std::make_unique<PIAna::PIPiDARFilter>("PiDAR", static_cast<int>(PIAna::EvtCode::PiDAR)));
  pi.add_action("DecPos", std::make_unique<PIAna::PITrueDecPos>("DecPos"));
  auto decpos = dynamic_cast<PIAna::PITrueDecPos *>(pi.get_action("DecPos"));
  // decpos->verbose(true);
  pi.add_action("RecHits", std::make_unique<PIAna::PIHitProducer>("RecHits"));
  auto rechits = dynamic_cast<PIAna::PIHitProducer *>(pi.get_action("RecHits"));
  // rechits->verbose(true);
  rechits->de_thres(1);
  pi.add_action("Topo", std::make_unique<PIAna::PITopoProducer>("Topo"));
  auto topoproducer =
      dynamic_cast<PIAna::PITopoProducer *>(pi.get_action("Topo"));
  topoproducer->min_tcluster(2);
  // topoproducer->verbose(true);
  pi.add_action("AccAna", std::make_unique<PIAna::PIAccAnalyzer>("AccAna"));
  auto accana = dynamic_cast<PIAna::PIAccAnalyzer *>(pi.get_action("AccAna"));
  accana->topoflag_name("Topo_flag");
  accana->pivertex_name("Topo_pivertex");
  accana->estart_name("Topo_estart");
  accana->edirection_name("Topo_edirection");
  accana->true_pivertex_name("DecPos_pivertex");
  accana->true_estart_name("DecPos_estart");
  pi.begin();
  pi.run();
  pi.end();
}

void test_pienu()
{
  run_pienu();
}
