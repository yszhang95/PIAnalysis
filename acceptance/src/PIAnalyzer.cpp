#include "PIAnalyzer.hpp"
#include "PIAnaEvtBase.hpp"
#include "PIAnaHit.hpp"

#include "PIAnaPat.hpp"
#include "PIMCInfo.hh"
#include "PIMCTrack.hh"

#include "TClonesArray.h"
#include "TChain.h"
#include "TError.h"
#include "TFile.h"
#include "TH1D.h"
#include "TTree.h"

#include <RtypesCore.h>
#include <iostream>
#include <algorithm>
#include <memory>
#include <type_traits>

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

PIAnalyzer::PIAnalyzer(const std::string& treename)
  : PIAnaEvtBase(treename), hi_nhits_(0),
pi_stop_xstrip_(-1), pi_stop_ystrip_(-1), pi_stop_zlayer_(-1),
pi_stop_found_(false)
{
  divider_ = std::make_unique<PIAnaG4StepDivider>();
  merger_ = std::make_unique<PIAnaHitMerger>();

  std::fill_n(hi_xstrip_, NHITS_MAX_, -1);
  std::fill_n(hi_ystrip_, NHITS_MAX_, -1);
  std::fill_n(hi_zlayer_, NHITS_MAX_, -1);
  std::fill_n(hi_edep_, NHITS_MAX_, -1);
  std::fill_n(hi_t_, NHITS_MAX_, -1E15);
}

PIAnalyzer::~PIAnalyzer()
{
}

void PIAnalyzer::begin()
{
  if (!PIAnaEvtBase::initialized_) {
    PIAnaEvtBase::initialize();
  }

  divider_->step_limit(0.01); // I assume it is in mm
  divider_->g4_step_limit(60); // I assume it is in mm

  merger_->dt_min(10); // I assum it is in ns

  t_ = new TTree("hi_hits", "hti_hits");
  t_->SetDirectory(PIAnaEvtBase::fout_.get());
  t_->Branch("hi_nhits", &hi_nhits_, "hi_nhits/s");
  t_->Branch("hi_xstrip", hi_xstrip_, "hi_xstrip[hi_nhits]/S");
  t_->Branch("hi_ystrip", hi_ystrip_, "hi_ystrip[hi_nhits]/S");
  t_->Branch("hi_zlayer", hi_zlayer_, "hi_zlayer[hi_nhits]/S");
  t_->Branch("hi_edep", hi_edep_, "hi_edep[hi_nhits]/F");
  t_->Branch("hi_t", hi_t_, "hi_t[hi_nhits]/F");
  t_->Branch("pi_stop_xstrip", &pi_stop_xstrip_, "pi_stop_xstrip/S");
  t_->Branch("pi_stop_ystrip", &pi_stop_ystrip_, "pi_stop_ystrip/S");
  t_->Branch("pi_stop_zlayer", &pi_stop_zlayer_, "pi_stop_zlayer/S");
  t_->Branch("pi_stop_found", &pi_stop_found_, "pi_stop_found/O");

  h_pi_all_ = new TH1D("h_pi_hits_all", "h_pi_hits_all;layer id;counts", 50,
                       -0.5, 49.5);
  h_pi_fake_ = new TH1D("h_pi_hits_fake", "h_pi_hits_fake;layer id;counts", 50,
                        -0.5, 49.5);
  h_pi_true_ = new TH1D("h_pi_hits_true", "h_pi_hits_true;layer id;counts", 50,
                        -0.5, 49.5);
  h_prompt_ = new TH1D("h_prompt", "prompt hits;pdgid;#Hits", 500, -0.5, 495.5);

  h_nonprompt_ =
    new TH1D("h_nonprompt", "nonprompt hits;pdgid;#Hits", 500, -0.5, 495.5);

  h_pi_all_->SetDirectory(PIAnaEvtBase::fout_.get());
  h_pi_fake_->SetDirectory(PIAnaEvtBase::fout_.get());
  h_pi_true_->SetDirectory(PIAnaEvtBase::fout_.get());
  h_prompt_->SetDirectory(PIAnaEvtBase::fout_.get());
  h_nonprompt_->SetDirectory(PIAnaEvtBase::fout_.get());

}

void PIAnalyzer::run()
{
  Long64_t ntotal = 0;
  Long64_t n_pi_stopped = 0;
  Long64_t n_pi_nostop = 0;
  Long64_t n_pi_stop_2 = 0;
  if (!initialized_) {
    std::cerr << "[ERROR] PIAnalyzer: Not initialized. "
                 "Call PIAnalyzer::begin() before PIAnalyzer::run()\n";
    return;
  }
  for (long long ientry = 0; ientry < chain_->GetEntriesFast(); ++ientry) {
    ntotal++;
    int code = analyze(ientry);
    switch (code) {
    case 0:
      n_pi_stopped++;
      break;
    case 1:
      n_pi_nostop++;
      break;
    case 2:
      // stop pion but tnp-tp < 5ns
      n_pi_stop_2++;
      break;
    case 3:
      // not hitting first layer
      break;
    }
  }
  std::cout << "Analyzed " << ntotal << " events.\n";
  std::cout << "#pion_stop deltaT>5ns " << n_pi_stopped << "\n";
  std::cout << "#pion_stop deltaT<5ns " << n_pi_stop_2 << "\n";
  std::cout << "#pion_nostop " << n_pi_nostop << "\n";
}

void PIAnalyzer::end()
{
  if (fout_) {
    fout_->Write();
  }
}

int PIAnalyzer::analyze(long long ientry)
{
  if (ientry % 1000 == 0) {
    std::cout << "Analyzing " << ientry << " events" << std::endl;
  }

  clear();
  auto itree = chain_->LoadTree(ientry);
  if (itree < 0) return -1;
  auto bytes = chain_->GetEntry(ientry);
  if (bytes <= 0) return -2;
  int atar_code = analyze_atar_hits();

  t_->Fill();

  return atar_code;
}

void PIAnalyzer::clear()
{
  hi_nhits_ = 0;
  std::fill_n(hi_xstrip_, NHITS_MAX_, -1);
  std::fill_n(hi_ystrip_, NHITS_MAX_, -1);
  std::fill_n(hi_zlayer_, NHITS_MAX_, -1);
  std::fill_n(hi_edep_, NHITS_MAX_, -1);
  std::fill_n(hi_t_, NHITS_MAX_, -1E15);

  pi_stop_xstrip_ = -1;
  pi_stop_ystrip_ = -1;
  pi_stop_zlayer_ = -1,
  pi_stop_found_ = false;
}

int PIAnalyzer::analyze_atar_hits()
{
  // pion must stop
  // KE < 1keV
  // a realistic value will be determined by minimum induced charges
  for (size_t j=0; j<track_->GetEntries(); ++j) {
    auto g4track = dynamic_cast<PIMCTrack*>(track_->At(j));
    // track;
    if (g4track->GetPDGID() == 211) {
      bool stopped = false;
      // assume they are sorted by time
      const auto& ts = g4track->GetPostTime();
      const auto &pxs = g4track->GetPostMomX();
      const auto &pys = g4track->GetPostMomY();
      const auto& pzs = g4track->GetPostMomZ();
      for (std::vector<Float_t>::size_type i = 0; i < ts.size(); ++i) {
        const auto t = ts.at(i);
        if (i > 0 && t - ts.at(i-1) < 0) {
          Error("PIAnalyzer::analyze_atar_hits",
                "The arrays of pion momentum are not sorted by time.");
        }
        const auto px = pxs.at(i);
        const auto py = pys.at(i);
        const auto pz = pzs.at(i);
        const auto m = 0.13957f;
        const auto e2 = px * px + py * py + pz * pz + m * m;
        const auto KE = std::sqrt(e2) - m;
        if (KE < 0.1) {
          stopped =true;
          break;
        }
      }
      if (!stopped) {
        // Info("PIAnalyzer::analyze_atar_hits", "Pion does not stop. Skip.");
        return 1;
      }
      // else {
      //   Info("PIAnalyzer::analyze_atar_hits", "Pion stops.");
      // }
    }
  }

  std::vector<PIAnaHit> rec_hits;
  double last_t = -1E15;
  int pi_stop_index = -1;
  for (int j=0; j<atar_->GetEntries(); ++j) {
    auto atar_hit = dynamic_cast<PIMCAtar*>(atar_->At(j));
    if (std::abs(atar_hit->GetPDGID()) == 22) {
      std::cout << "Event ID " << info_->GetRun() << ":"
                << info_->GetEvent()
                << ":"
                << info_->GetEventID();
      std::cout << ". Photon with track label " << atar_hit->GetTrackID()
                << " detected in ATAR. Its energy deposition is "
                << atar_hit->GetEdep() << " in pxl "
                << atar_hit->GetPixelID() << ".\n";
    }
    auto hits = divider_->process_atar_hit(*atar_hit);
    auto merged_hits = merger_->merge(hits);

    std::move(merged_hits.begin(), merged_hits.end(),
              std::back_inserter(rec_hits));

    if (atar_hit->GetPDGID() == 211) {
      last_t = last_t < atar_hit->GetTime() ? atar_hit->GetTime() : last_t;
      pi_stop_index = j;
    }
  }

  rec_hits = merger_->merge(rec_hits);

  for (std::vector<PIAnaHit>::size_type i=0; i<rec_hits.size(); ++i) {
    const auto& hit = rec_hits.at(i);
    // cheating here
    // mu- == 13, e- = 11, pi+ == 211
    if (hit.pdgid() == 211 || std::abs(hit.pdgid()) == 13) {
      hi_xstrip_[hi_nhits_] = hit.xstrip();
      hi_ystrip_[hi_nhits_] = hit.ystrip();
      hi_zlayer_[hi_nhits_] = hit.layer();
      hi_edep_[hi_nhits_] = hit.edep();
      hi_t_[hi_nhits_] = hit.t();
      ++hi_nhits_;
    }
  }

  if (pi_stop_index >=0) {
    auto pi_stop_hit = dynamic_cast<PIMCAtar*>(atar_->At(pi_stop_index));
    // PIAnaHit::find_strip(pi_stop_hit->GetX1());
    // PIAnaHit::find_strip(pi_stop_hit->GetY1());
    // PIAnaHit::find_layer(pi_stop_hit->GetZ1());

    auto hits = divider_->process_atar_hit(*pi_stop_hit);
    auto merged_hits = merger_->merge(hits);
    std::sort(merged_hits.begin(), merged_hits.end(),
              [](PIAnaHit const& h1, PIAnaHit const& h2)
                ->bool { return h1.t() < h2.t(); });

    pi_stop_xstrip_ = merged_hits.back().xstrip();
    pi_stop_ystrip_ = merged_hits.back().ystrip();
    pi_stop_zlayer_ = merged_hits.back().layer();
    pi_stop_found_ = true;
  }
  // prompt cluster
  PIAnaPat pat;
  pat.verbose(true);
  std::vector<PIAnaHit const*> hit0s;
  for (const auto &hit : rec_hits) {
    if (hit.layer() == 0) {
      hit0s.push_back(&hit);
    }
  }
  if (hit0s.empty()) {
    return 3;
  }
  std::sort(std::begin(hit0s), std::end(hit0s),
      [](const PIAnaHit *h1, const PIAnaHit *h2) { return h1->t() < h2->t(); });
  pat.t0(hit0s.front()->t());

  // int pat_code = pat.process_event(rec_hits);
  int pat_code = pat.process_event(rec_hits.begin(), rec_hits.end());

  //  std::cout << "Processed " << t_->GetEntries() << " events" << std::endl;
  if (pat_code !=0 )
    return pat_code;

  auto loc_cluster = pat.local_cluster() ;
  //  const auto& pi_fitter = loc_cluster->pi_fitter();
  //  auto  pi_line = pi_fitter.graphics();
  //  const auto& e_fitter = loc_cluster->e_fitter();
  //  auto  e_line = e_fitter.graphics();

  // reco
  const auto &prompt_hits = loc_cluster->prompt_cluster();
  const auto& nonprompt_hits = loc_cluster->nonprompt_cluster();
  for (const auto hit : prompt_hits) {
    bool fake = true;
    for (const auto pdgid : hit->pdgids()) {
      if (std::abs(pdgid) == 211) {
        fake = false;
        break;
      }
    }
    if (fake) {
      h_pi_fake_->Fill(hit->layer());
      // std::cout << "Fake";
      // for (const auto hit2 : prompt_hits) {
      //   std::cout << "\t"  << hit2->pdgid() << "("<< hit2->t() << ")";
      // }
      // std::cout << " Predecessor ended"<< std::endl;
      // for (const auto pdgid : hit->pdgids()) {
      //   std::cout << "\t" << pdgid << "(" << hit->t() << ")";
      // }
      // std::cout << "\tNPHit";
      // for (const auto np_hit : nonprompt_hits) {
      //   std::cout << "\t" << np_hit->pdgid() << "(" << np_hit->t() << ")";
      // }
      // std::cout << std::endl;
    }
    else
      h_pi_true_->Fill(hit->layer());

    h_prompt_->Fill(std::abs(hit->pdgid()));
  }

  for (const auto hit : nonprompt_hits) {
    h_nonprompt_->Fill(std::abs(hit->pdgid()));
  }

  // truth
  for (const auto& hit : rec_hits) {
    bool pion = false;
    for (const auto pdgid : hit.pdgids()) {
      if (std::abs(pdgid) == 211) {
        pion = true;
        break;
      }
    }
    if (pion)
      h_pi_all_->Fill(hit.layer());
  }
  return 0;
}

ClassImp(PIAnalyzer)
