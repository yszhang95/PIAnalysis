#include <Rtypes.h>
#ifdef __CLING__
R__LOAD_LIBRARY(../../install/lib/libPiAnaAcc.so)
R__LOAD_LIBRARY(../../install/lib/libPiRootDict.so)
#else
#include "PIAnaAtarPho.hpp"
#endif
void test_atarpho()
{
  PIAnaAtarPho pi("sim");
  // pi.verbose(true);
  pi.add_file("pienu00000-00.root");
  // pi.filter("0:602:602");
  pi.begin();
  pi.run();
  pi.end();
}
