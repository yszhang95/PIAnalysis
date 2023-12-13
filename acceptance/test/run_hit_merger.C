#include "TSystem.h"
#include "TClonesArray.h"
#include "TGraph2D.h"
#include "TFile.h"
#include "TTree.h"

#include <iostream>
#include <iterator>

#ifndef __CLING__
#include "PIAnaHit.hpp"
#include "PIAnaPat.hpp"
#include "PIMCAtar.hh"
#else
R__LOAD_LIBRARY(../../install/lib/libPiRootDict.dylib)
R__LOAD_LIBRARY(../../install/lib/libPiAnaAcc.dylib)
#endif

void print_hit(const PIAnaHit& hit)
{
  std::cout << "layer: "<< hit.layer()
  << "\txstrip: " << hit.xstrip()
  << "\tystrip: " << hit.ystrip()
  << "\tt: " << hit.t() << "\t";
  std::cout << "z: " << hit.z() << "\t";
  std::cout << "rec_z: " << hit.rec_z() << "\t";
  // std::cout << "dE/dz: " << rec_hits.at(i).edep() / 0.012 << "\t";
  std::cout << "pdgid: " << hit.pdgid() << " pdgids: ";
  for (const auto id : hit.pdgids()) {
    std::cout << id << "\t";
  }
  std::cout <<  "\n";
}

void run_hit_merger(Long64_t entry=-1, const bool printout=true)
{
  // plotter
  TGraph2D* pi_rec_plot = nullptr;
  TGraph2D* pi_gen_plot = nullptr;

  TGraph2D* e_rec_plot = nullptr;
  TGraph2D* e_gen_plot = nullptr;

  PIAnaG4StepDivider divider;
  divider.step_limit(0.01); // I assume it is in mm
  divider.g4_step_limit(60); // I assume it is in mm

  PIAnaHitMerger merger;
  merger.dt_min(1); // I assum it is in ns

  PIAnaPat patern;
  patern.verbose(true);

  TClonesArray *fAtar = new TClonesArray("PIMCAtar");

  auto fout = TFile("output.root", "recreate");

  // auto f = TFile::Open("doubleside_w_sens_det.root");
  auto f = TFile::Open("pienu00000-00.root");

  auto t = (TTree*)f->Get("sim");

  t->SetBranchAddress("atar", &fAtar);

  Int_t nEntries = t->GetEntries();
  for (Int_t ientry = 0; ientry < nEntries; ++ientry) {

    if (entry>=0 && entry!=ientry) continue;
    t->GetEntry(ientry);
    if (entry >=0 ) {
      std::cout << "Loaded " << ientry << "th event\n";
    }

    // Here you have full access to all data members of the event.
    // Speed up by only reading certain branches, up to the user.

    std::vector<PIMCAtar> gen_hits;
    std::vector<PIAnaHit> rec_hits;
    gen_hits.reserve(100);
    rec_hits.reserve(100);

    std::vector<PIMCAtar> pi_gen_hits;
    std::vector<PIAnaHit> pi_rec_hits;

    std::vector<PIMCAtar> e_gen_hits;
    std::vector<PIAnaHit> e_rec_hits;

    for (size_t j = 0; j < fAtar->GetEntries(); ++j) {
      auto atar_hit = dynamic_cast<PIMCAtar*>(fAtar->At(j));
      auto hits = divider.process_atar_hit(*atar_hit);

      // std::cout << "hits: " << hits.front().z() << "\t" << hits.back().z() << "\n";

      // if (hits.empty()) { std::cout << "empty\n"; }
      auto merged_hits = merger.merge(hits);
      // for (int i=0; i<merged_hits.size(); ++i) {
      //   std::cout << merged_hits.at(i).layer() << "\n";
      // }
      // rec_hits.assign(std::back_inserter(rec_hits));
      std::move(merged_hits.begin(), merged_hits.end(),
                std::back_inserter(rec_hits));

      // std::cout << "\n" << j << "th event\n";
      for (int i=0; i<merged_hits.size(); ++i) {
        if (!printout) break;
        const auto& hit = merged_hits.at(i);
      }
      if (atar_hit) {
        if (!printout) break;
        std::cout << "\n";
        gen_hits.push_back(*atar_hit);
        std::cout << "gen step" << gen_hits.size()-1 << "\t"
        << "z0: " << atar_hit->GetZ0() << "\t"
        << "z1: " << atar_hit->GetZ1() << "\t"
        << "pdgid: " << atar_hit->GetPDGID()
        << "\n";
      }
    }

    rec_hits = merger.merge(rec_hits);

    if (printout) {
      for(const auto &hit : rec_hits) {
        print_hit(hit);
      }
    }

    if (!rec_hits.empty()) {
      patern.process_event(rec_hits);
    }
    else {
      std::cout << "[LOG] No hits found in this event.\n";
    }

    if (entry >= 0 && !gen_hits.empty()) {
      for (const auto & hit : gen_hits) {
        if (hit.GetPDGID() == -11) {
          e_gen_hits.push_back(hit);
        } else if (hit.GetPDGID() == 211) {
          pi_gen_hits.push_back(hit);
        }
      }

      // std::cout << "positron size: " << e_gen_hits.size() << "\n";

      for (const auto & hit: rec_hits) {
        if (hit.pdgid() == -11) {
          e_rec_hits.push_back(hit);
        } else if (hit.pdgid() == 211) {
          pi_rec_hits.push_back(hit);
        }
      }

      delete pi_rec_plot;
      delete pi_gen_plot;
      delete e_rec_plot;
      delete e_gen_plot;

      if (!pi_gen_hits.empty()) {
        pi_gen_plot = new TGraph2D(2*pi_gen_hits.size());
        pi_gen_plot->SetName(::Form("pi_gen_event%lld", entry));
      }

      if (!pi_rec_hits.empty()) {
        pi_rec_plot = new TGraph2D(pi_rec_hits.size());
        pi_rec_plot->SetName(::Form("pi_rec_event%lld", entry));
      }

      if (!e_rec_hits.empty()) {
        e_rec_plot = new TGraph2D(e_rec_hits.size());
        e_rec_plot->SetName(::Form("e_rec_event%lld", entry));
      }

      if (!e_gen_hits.empty()) {
        e_gen_plot = new TGraph2D(2*e_gen_hits.size());
        e_gen_plot->SetName(::Form("e_gen_event%lld", entry));
      }

      for (std::vector<PIAnaHit>::size_type i=0; i<e_rec_hits.size(); ++i) {
        const auto& hit = e_rec_hits.at(i);
        e_rec_plot->SetPoint(i, hit.rec_x(), hit.rec_y(), hit.rec_z());
      }

      for (std::vector<PIAnaHit>::size_type i=0; i<pi_rec_hits.size(); ++i) {
        const auto& hit = pi_rec_hits.at(i);
        pi_rec_plot->SetPoint(i, hit.rec_x(), hit.rec_y(), hit.rec_z());
      }


      for (std::vector<PIMCAtar>::size_type i=0; i<e_gen_hits.size(); ++i) {
        const auto& hit = e_gen_hits.at(i);
        e_gen_plot->SetPoint(2*i, hit.GetX0(), hit.GetY0(), hit.GetZ0());
        e_gen_plot->SetPoint(2*i+1, hit.GetX1(), hit.GetY1(), hit.GetZ1());
      }
    }
    for (std::vector<PIMCAtar>::size_type i=0; i<pi_gen_hits.size(); ++i) {
      const auto& hit = pi_gen_hits.at(i);
      pi_gen_plot->SetPoint(2*i, hit.GetX0(), hit.GetY0(), hit.GetZ0());
      pi_gen_plot->SetPoint(2*i+1, hit.GetX1(), hit.GetY1(), hit.GetZ1());
    }
  }

  if (e_rec_plot) {
    fout.cd();
    e_rec_plot->Write(::Form("e_rec_event%lld", entry));
  }

  if (pi_rec_plot) {
    fout.cd();
    pi_rec_plot->Write(::Form("pi_rec_event%lld", entry));
  }

  if (e_gen_plot) {
    fout.cd();
    e_gen_plot->Write(::Form("e_gen_event%lld", entry));
  }

  if (pi_gen_plot) {
    fout.cd();
    pi_gen_plot->Write(::Form("pi_gen_event%lld", entry));
  }

  delete f;
}
