#include <Rtypes.h>
#ifdef __CLING__
R__LOAD_LIBRARY(../../../PIAnalysis_install/lib/libPiAnaAcc.so)
R__LOAD_LIBRARY(../../../PIAnalysis_install/lib/libPiRootDict.so)
#else
#include "PIEvtNbFilter.hpp"
#include "PIAnaConst.hpp"
#include "PIFilterPiDAR.hpp"
#include "PITrueMomProducer.hpp"
#include "PIJobManager.hpp"
#endif

void run_test_pienu()
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
  pi.add_action("TrueEMom", std::make_unique<PIAna::PITrueMomProducer>("TrueMom"));
  auto emom = dynamic_cast<PIAna::PITrueMomProducer *>(pi.get_action("TrueEMom"));
  emom->decay_mode(PIAna::PITrueMomProducer::PiDecayMode::pienu);
  emom->verbose(true);
  pi.begin();
  pi.run();
  pi.end();
}

void run_test_pimue()
{
  PIAna::PIJobManager pi;
  pi.treename("sim");
  pi.add_file("pimunu-test.root");
  pi.out_file("pimue-job-output.root");
  pi.add_action("PiDAR",
                std::make_unique<PIAna::PIPiDARFilter>("PiDAR", static_cast<int>(PIAna::EvtCode::PiDAR)));
  pi.add_action("DecPos", std::make_unique<PIAna::PITrueDecPos>("DecPos"));
  auto decpos = dynamic_cast<PIAna::PITrueDecPos *>(pi.get_action("DecPos"));
  // decpos->verbose(true);
  pi.add_action("TrueEMom", std::make_unique<PIAna::PITrueMomProducer>("TrueMom"));
  auto emom = dynamic_cast<PIAna::PITrueMomProducer *>(pi.get_action("TrueEMom"));
  emom->decay_mode(PIAna::PITrueMomProducer::PiDecayMode::pimue);
  emom->verbose(true);
  pi.begin();
  pi.run();
  pi.end();
}

void test_decay_mom()
{
  run_test_pienu();
  run_test_pimue();
}
