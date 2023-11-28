#include "TSystem.h"
#include "TClonesArray.h"
#include "TFile.h"
#include "TTree.h"
#include <iterator>

#ifndef __CLING__
#include "PIAnaHit.hpp"
#include "PIMCAtar.hh"
#else
R__LOAD_LIBRARY(../../install/lib/libPiRootDict.dylib)
R__LOAD_LIBRARY(../../install/lib/libPiAnaAcc.dylib)
#endif

void run_hit_merger()
{
//  gSystem->Load("../../install/lib/libPiAnaAcc.dylib");
  PIAnaG4StepDivider divider;
  // divider.step_limit(0.01); // I assume it is in mm
  // divider.g4_step_limit(0.2); // I assume it is in mm
  divider.step_limit(0.1); // I assume it is in mm
  divider.g4_step_limit(6); // I assume it is in mm

  PIAnaHitMerger merger;
  merger.dt_min(20); // I assum it is in ns

  TClonesArray *fAtar = new TClonesArray("PIMCAtar");

  auto f = TFile::Open("doubleside_w_sens_det.root");

  auto t = (TTree*)f->Get("sim");

  t->SetBranchAddress("atar", &fAtar);

  Int_t nEntries = t->GetEntries();
  for (Int_t i = 0; i < nEntries; ++i) {
    t->GetEntry(i);
    // Here you have full access to all data members of the event.
    // Speed up by only reading certain branches, up to the user.

    std::vector<PIAnaHit> rec_hits;
    rec_hits.reserve(100);

    for (size_t j = 0; j < fAtar->GetEntries(); ++j) {
      auto atar_hit = dynamic_cast<PIMCAtar*>(fAtar->At(j));
      auto hits = divider.process_atar_hit(*atar_hit);
      // if (hits.empty()) { std::cout << "empty\n"; }
      auto merged_hits = merger.merge(hits);
      // for (int i=0; i<merged_hits.size(); ++i) {
      //   std::cout << merged_hits.at(i).layer() << "\n";
      // }
      // rec_hits.assign(std::back_inserter(rec_hits));
      std::move(merged_hits.begin(), merged_hits.end(),
                std::back_inserter(rec_hits));

      std::cout << "\n" << j << "th event\n";
      for (int i=0; i<rec_hits.size(); ++i) {
        std::cout << "layer: "<< rec_hits.at(i).layer() << "\t";
        std::cout << "dE/dz: " << rec_hits.at(i).edep() / 0.012 << "\t";
        std::cout << "pdgid: " << rec_hits.at(i).pdgid() << " pdgids: ";
        for (const auto id : rec_hits.at(i).pdgids()) {
          std::cout << id << "\t";
        }
        std::cout <<  "\n";
      }
    }
  }

  delete f;
}
