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
#include "PIJobManager.hpp"
#include "PITreeAnalyzer.hpp"
#endif

void run_test_pienu()
{
  PIAna::PIJobManager pi;
  pi.treename("sim");
  pi.add_file("pienu-test.root");
  pi.out_file("pienu-job-output.root");
  std::unique_ptr<PIAna::PIEvtNbFilter> evtnbfilter =
      std::make_unique<PIAna::PIEvtNbFilter>("EvtNb", 0);
  evtnbfilter->load_filter("0:4105:4105,0:9805:9805");
  pi.add_action("EvtNb", std::move(evtnbfilter));
  auto evtnbfilterptr = dynamic_cast<PIAna::PIEvtNbFilter*>(pi.get_action("EvtNb"));
  evtnbfilterptr->load_filter("0:9599:9599");
  pi.add_action("PiDAR",
                std::make_unique<PIAna::PIPiDARFilter>("PiDAR", static_cast<int>(PIAna::EvtCode::PiDAR)));
  pi.add_action("DecPos", std::make_unique<PIAna::PITrueDecPos>("DecPos"));
  auto decpos = dynamic_cast<PIAna::PITrueDecPos *>(pi.get_action("DecPos"));
  decpos->verbose(true);
  pi.add_action("RecHits", std::make_unique<PIAna::PIHitProducer>("RecHits"));
  auto rechits = dynamic_cast<PIAna::PIHitProducer *>(pi.get_action("RecHits"));
  rechits->verbose(true);
  pi.add_action("Topo", std::make_unique<PIAna::PITopoProducer>("Topo"));
  auto topoproducer =
      dynamic_cast<PIAna::PITopoProducer *>(pi.get_action("Topo"));
  topoproducer->min_tcluster(2);
  topoproducer->verbose(true);
  std::unique_ptr<PIAna::PITreeAnalyzer> treeanalyzer =
      std::make_unique<PIAna::PITreeAnalyzer>("AnaHits");
  pi.add_action("AnaHits", std::move(treeanalyzer));
  pi.begin();
  pi.run();
  pi.end();
}

void test_job()
{
  run_test_pienu();
}
