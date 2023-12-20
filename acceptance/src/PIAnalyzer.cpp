#include "PIAnalyzer.hpp"
#include "PIAnaHit.hpp"

#include "PIMCInfo.hh"
#include "PIMCTrack.hh"

#include "TClonesArray.h"
#include "TChain.h"
#include "TFile.h"
#include "TTree.h"

#include <algorithm>
#include <memory>

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
: treename_(treename), hi_nhits_(0),
pi_stop_xstrip_(-1), pi_stop_ystrip_(-1), pi_stop_zlayer_(-1),
pi_stop_found_(false), initialized_(false)
{
  chain_ = std::make_unique<TChain>(treename.c_str());

  divider_ = std::make_unique<PIAnaG4StepDivider>();
  merger_ = std::make_unique<PIAnaHitMerger>();

  info_ = new PIMCInfo();
  atar_ = new TClonesArray("PIMCAtar");
  track_ = new TClonesArray("PIMCTrack");

  chain_->SetBranchAddress("info", &info_);
  chain_->SetBranchAddress("atar", &atar_);
  chain_->SetBranchAddress("track", &track_);

  std::fill_n(hi_xstrip_, NHITS_MAX_, -1);
  std::fill_n(hi_ystrip_, NHITS_MAX_, -1);
  std::fill_n(hi_zlayer_, NHITS_MAX_, -1);
  std::fill_n(hi_edep_, NHITS_MAX_, -1);
  std::fill_n(hi_t_, NHITS_MAX_, -1E15);
}

PIAnalyzer::~PIAnalyzer()
{
  delete info_;
  delete atar_;
  delete track_;
}

void PIAnalyzer::add_file(const std::string& f)
{
  if (!initialized_) {
    filenames_.push_back(f);
  }
}

void PIAnalyzer::add_friend(const std::string& ftree)
{
  if (!initialized_) {
    ftreenames_.push_back(ftree);
  }
}

void PIAnalyzer::begin()
{
  for (const auto& ftname : ftreenames_) {
    chain_->AddFriend(ftname.c_str());
  }

  for (const auto& fname : filenames_) {
    chain_->Add(fname.c_str());
  }

  divider_->step_limit(0.01); // I assume it is in mm
  divider_->g4_step_limit(60); // I assume it is in mm

  merger_->dt_min(10); // I assum it is in ns

  fout_ = std::make_unique<TFile>("output.root", "recreate");

  t_ = new TTree("hi_hits", "hti_hits");
  t_->SetDirectory(fout_.get());
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

  initialized_ = true;
}

void PIAnalyzer::run()
{
  if (!initialized_) {
    std::cerr << "[ERROR] PIAnalyzer: Not initialized. "
                 "Call PIAnalyzer::begin() before PIAnalyzer::run()\n";
    return;
  }
  for (long long ientry=0; ientry<chain_->GetEntriesFast(); ++ientry) {
    analyze(ientry);
  }
}

void PIAnalyzer::end()
{
  if (fout_) {
    fout_->Write();
  }
}

void PIAnalyzer::analyze(long long ientry)
{
  clear();
  auto itree = chain_->LoadTree(ientry);
  if (itree < 0) return;
  auto bytes = chain_->GetEntry(ientry);
  if (bytes <= 0) return;
  analyze_atar_hits();

  t_->Fill();
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

void PIAnalyzer::analyze_atar_hits()
{
  // for (size_t j=0; j<track_->GetEntries(); ++j) {
  //   auto g4track = dynamic_cast<PIMCTrack*>(track_->At(j));
  //   // track;
  //   if (g4track->GetPDGID() == 211) {
  //     PIAnaHit::find_strip(g4track->GetPostX().back());
  //     PIAnaHit::find_strip(g4track->GetPostY().back());
  //     PIAnaHit::find_layer(g4track->GetPostZ().back());
  //   }
  // }

  std::vector<PIAnaHit> rec_hits;
  double last_t = -1E15;
  int pi_stop_index = -1;
  for (int j=0; j<atar_->GetEntries(); ++j) {
    auto atar_hit = dynamic_cast<PIMCAtar*>(atar_->At(j));

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
}

ClassImp(PIAnalyzer)
