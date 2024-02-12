#include "PIAnaAtarPho.hpp"
#include "PIAnaEvtBase.hpp"

#include "TChain.h"
#include "TClonesArray.h"
#include "TFile.h"
#include "TH1F.h"

#include "PIMCInfo.hh"
#include "PIMCAtar.hh"
#include "PIMCTrack.hh"
#include <type_traits>

PIAnaAtarPho::PIAnaAtarPho(const std::string &treename)
    : PIAnaEvtBase(treename)
{

}

PIAnaAtarPho::~PIAnaAtarPho() {}

void PIAnaAtarPho::begin()
{
  PIAnaEvtBase::initialize();
}


void PIAnaAtarPho::run()
{
  if (!initialized_) {
    std::cerr << "[ERROR] PIAnaAtarPho: Not initialized. "
                 "Call PIAnaAtarPho::begin() before PIAnaAtarPho::run()\n";
    return;
  }
  analyze();
}

void PIAnaAtarPho::end()
{
  if (procs_.empty())
    return;
  auto nproc = procs_.size();
  fout_->cd();
  TH1F *h1 =
    new TH1F("hPhoProc", "Processes of interactions between photons and ATAR",
             nproc, 0, nproc);
  TH1F *h2 = new TH1F("hPhoProcEdep",
                      "Processes of the energy loss in ATAR for photons",
                      nproc, 0, nproc);

  h1->SetDirectory(fout_.get());
  h2->SetDirectory(fout_.get());
  std::map<int, unsigned long long>::size_type i = 0;
  if (verbose_) {
    std::cout << "[INFO] Processes of the interaction between photons and ATAR.\n";
    for (const auto proc : procs_) {
      std::cout << "[INFO] Process " << proc.first << ":\t " << proc.second
                << ".\n";
    }
    std::cout << "[INFO] Processes of the energy loss in ATAR for photons.\n";
    for (const auto proc : edep_procs_) {
      std::cout << "[INFO] Process "
                << proc.first << ":\t " << proc.second << ".\n";
    }
  }

  for (const auto proc : procs_) {
    h1->GetXaxis()->SetBinLabel(i + 1, std::to_string(proc.first).c_str());
    h1->SetBinContent(i + 1, proc.second);
    h2->GetXaxis()->SetBinLabel(i + 1, std::to_string(proc.first).c_str());
    if (edep_procs_.find(proc.first) != edep_procs_.end()) {
      h2->SetBinContent(i + 1, edep_procs_.at(proc.first));
    }
    i++;
  }
  h1->ResetStats();
  h2->ResetStats();
  std::cout << "[INFO] Analyzed " << h1->GetEntries() << " hits.\n";
  std::cout << "[INFO] Energy depositions are observed "
            << h2->GetEntries()
            << " times. (A threshold is set to 1E-10.)\n";
  h1->Write();
  h2->Write();
}

int PIAnaAtarPho::analyze()
{
  for (Long64_t ientry = 0; ientry < chain_->GetEntriesFast(); ++ientry) {
    if (ientry % 1000 == 0) {
      std::cout << "[INFO] Processes " << ientry << " events.\n";
    }

    auto itree = chain_->LoadTree(ientry);
    if (itree < 0)
      return -1;

    auto bytes = chain_->GetEntry(ientry);
    if (bytes <= 0)
      return -2;

    if (!select_event_id_(info_->GetRun(),
                          info_->GetEvent(), info_->GetEventID()))
      continue;

    analyze_atarpho();
  }
  return 0;
}

int PIAnaAtarPho::analyze_atarpho()
{
  for (size_t j = 0; j < track_->GetEntries(); ++j) {
    auto g4track = dynamic_cast<PIMCTrack*>(track_->At(j));
    // track;
    if (std::abs(g4track->GetPDGID()) == 22) {
      // assume they are sorted by time
      // const auto& ts = g4track->GetPostTime();
      // const auto &pxs = g4track->GetPostMomX();
      // const auto &pys = g4track->GetPostMomY();
      // const auto &pzs = g4track->GetPostMomZ();
      // g4track->GetParentProcess();
      const auto &edeps = g4track->GetEdep();
      const auto &vols = g4track->GetVolume();
      const auto &processes = g4track->GetProcessID();
      for (std::vector<Int_t>::size_type i = 0; i < vols.size(); ++i) {
        if (vols.at(i) >= 180000 && vols.at(i) <= 189999) {
          if (verbose_) {
            std::cout << "[INFO] Silcon bulk volume ID " << vols.at(i)
                      << " interacts with a photon "
                         "under process "
                      << processes.at(i)
                      << ". Energy deposition is " << edeps.at(i) <<".\n";
          }
          // counting
          auto it = procs_.find(processes.at(i));
          if (it == procs_.end()) {
            procs_.insert({processes.at(i), 1});
          } else {
            ++(it->second);
          }
          // counting
          auto edep_it = edep_procs_.find(processes.at(i));
          if (edep_it == edep_procs_.end() && edeps.at(i) > 1E-10) {
              edep_procs_.insert({processes.at(i), 1});
          }
          if (edep_it != edep_procs_.end() && edeps.at(i) > 1E-10){
              ++(edep_it->second);
          }
        }
      }
    }
  }
  return 0;
}

ClassImp(PIAnaAtarPho)
