#include <Rtypes.h>
#ifdef __CLING__
R__LOAD_LIBRARY(../../../PIAnalysis_install/lib/libPiAnaAcc.so)
R__LOAD_LIBRARY(../../../PIAnalysis_install/lib/libPiRootDict.so)
#else
#include "PIAnaConst.hpp"
#include "PIFilterPiDAR.hpp"
#include "PIHitProducer.hpp"
#include "PIJobManager.hpp"
#endif

void run_test_pienu()
{
    PIAna::PIJobManager pi;
  pi.treename("sim");
  pi.add_file("pienu-test.root");
  pi.out_file("pienu-job-output.root");
  pi.add_action("PiDAR", std::make_unique<PIAna::PIPiDARFilter>(
                             "PiDAR", static_cast<int>(PIAna::EvtCode::PiDAR)));
  pi.add_action("RecHits", std::make_unique<PIAna::PIHitProducer>("RecHits"));
  pi.begin();
  pi.run();
  pi.end();
}

void test_job()
{
  run_test_pienu();
}
