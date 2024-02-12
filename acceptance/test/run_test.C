#include <Rtypes.h>
#ifdef __CLING__
R__LOAD_LIBRARY(../../install/lib/libPiAnaAcc.so)
R__LOAD_LIBRARY(../../install/lib/libPiRootDict.so)
#else
#include "PIAnalyzer.hpp"
#endif
void run_test_pimunu()
{
  PIAnalyzer pi("sim");
  // pi.verbose(true);
  pi.add_file("pimunu-test.root");
  pi.out_file("pimunu-output.root");
  pi.filter("0:9599:9599");
  pi.min_tcluster(3);
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
  // pi.filter("0:850:850");
  pi.min_tcluster(2);
  pi.begin();
  pi.run();
  pi.end();
}

void run_test()
{
  run_test_pienu();
  run_test_pimunu();
}
