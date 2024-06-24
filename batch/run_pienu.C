#ifdef __CLING__
R__LOAD_LIBRARY(libPiAnaAcc.so)
R__LOAD_LIBRARY(libPiRootDict.so)
#else
#include "Rtypes.h"
#include "TError.h"
#include "TSystem.h"
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

#include "utils.C"

void run_pienu(std::string inputfile)
{
  gErrorIgnoreLevel = kFatal;
  PIAna::PIJobManager pi;
  pi.treename("sim");
  const auto filenames = load_filenames(inputfile, true);
  for (const auto& f : filenames) {
    pi.add_file(f);
  }
  TString hash = get_hash_input(inputfile);
  pi.out_file(::Form("pienu_job_output_%s.root", hash.Data()));
  pi.add_action("PiDAR",
                std::make_unique<PIAna::PIPiDARFilter>("PiDAR", static_cast<int>(PIAna::EvtCode::PiDAR)));
  pi.add_action("DecPos", std::make_unique<PIAna::PITrueDecPos>("DecPos"));
  auto decpos = dynamic_cast<PIAna::PITrueDecPos *>(pi.get_action("DecPos"));
  // decpos->verbose(1);
  pi.add_action("TrueEMom", std::make_unique<PIAna::PITrueMomProducer>("TrueEMom"));
  auto emom = dynamic_cast<PIAna::PITrueMomProducer *>(pi.get_action("TrueEMom"));
  emom->decay_mode(PIAna::PITrueMomProducer::PiDecayMode::pienu);
  // emom->verbose(1);
  pi.add_action("RecHits", std::make_unique<PIAna::PIHitProducer>("RecHits"));
  auto rechits = dynamic_cast<PIAna::PIHitProducer *>(pi.get_action("RecHits"));
  rechits->de_thres(0.0155);
  // rechits->verbose(1);
  pi.add_action("Topo", std::make_unique<PIAna::PITopoProducer>("Topo"));
  auto topoproducer =
      dynamic_cast<PIAna::PITopoProducer *>(pi.get_action("Topo"));
  topoproducer->min_tcluster(2);
  // topoproducer->verbose(1);
  pi.add_action("AccAna", std::make_unique<PIAna::PIAccAnalyzer>("AccAna"));
  auto accana = dynamic_cast<PIAna::PIAccAnalyzer *>(pi.get_action("AccAna"));
  accana->topoflag_name("Topo_flag");
  accana->pivertex_name("Topo_pivertex");
  accana->estart_name("Topo_estart");
  accana->edirection_name("Topo_edirection");
  accana->ehits_name("Topo_ehits");
  accana->true_pivertex_name("DecPos_pivertex");
  accana->true_estart_name("DecPos_estart");
  accana->etruemom_name("TrueEMom_emom");
  pi.begin();
  pi.run();
  pi.end();
}

