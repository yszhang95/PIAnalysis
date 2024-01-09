#include <Rtypes.h>
#ifdef __CLING__
R__LOAD_LIBRARY(../../install/lib/libPiAnaAcc.so)
R__LOAD_LIBRARY(../../install/lib/libPiRootDict.so)
#else
#include "PIAnalyzer.hpp"
#endif
void run_test()
{
  PIAnalyzer pi("sim");
  pi.add_file("pienu00000-00.root");
  pi.begin();
  pi.run();
  pi.end();
}
