#include <Rtypes.h>
#ifdef __CLING__
R__LOAD_LIBRARY(../../../PIAnalysis_install/lib/libPiAnaAcc.so)
R__LOAD_LIBRARY(../../../PIAnalysis_install/lib/libPiRootDict.so)
#else
#include "PIAnaConst.hpp"
#include "PIPiDARFilter.hpp"
#include "PIPiDecayInSi.hpp"
#include "PIAnalyzer.hpp"
#endif
void run_test_pimunu()
{
  PIAnalyzer pi("sim");
  // pi.verbose(true);
  pi.add_file("pimunu-test.root");
  pi.out_file("pimunu-output.root");
  // pi.filter("0:9599:9599");
  pi.min_tcluster(3);
  pi.pi_dt1(1);
  pi.merge_hit_dtmin(1);
  pi.tcluster_dt(5);
  pi.add_filter("PiDAR", std::make_unique<PIPiDARFilter>(
                             static_cast<int>(PIAna::EvtCode::PiDAR)));
  pi.add_filter("PiDecayInSi", std::make_unique<PIPiDecayInSi>(2, 180000,
                                                               185000));

  pi.begin();
  pi.run();
  pi.end();
}

void run_test_pienu()
{
  PIAnalyzer pi("sim");
  // pi.verbose(true);
  pi.add_file("pienu-test.root");
  pi.out_file("pienu-output.root");
  pi.filter("0:4105:4105");
  // pi.filter("0:9805:9805");
  pi.min_tcluster(2);
  pi.pi_dt1(1);
  pi.merge_hit_dtmin(1);
  pi.tcluster_dt(5);
  pi.add_filter("PiDAR", std::make_unique<PIPiDARFilter>(
                             static_cast<int>(PIAna::EvtCode::PiDAR)));
  pi.add_filter("PiDecayInSi", std::make_unique<PIPiDecayInSi>(2, 180000,
                                                               185000));
  // pi.verbose(true);
  pi.debug(true);
  pi.begin();
  pi.run();
  pi.end();
}

void run_test()
{

  // run_test_pimunu();
  run_test_pienu();
}
