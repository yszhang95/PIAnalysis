#include "PIAnalyzer.hpp"
#include "PIAnaEvtBase.hpp"
#include "PIAnaHit.hpp"

#include "PIAnaCluster.hpp"
#include "PIMCInfo.hh"
#include "PIMCTrack.hh"
#include "PIMCDecay.hh"

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

PIAnalyzer::PIAnalyzer(const std::string &treename)
    : PIAnaEvtBase(treename), hcategories_(nullptr), htcluster_(nullptr),
      min_tcluster_(1)
{
  divider_ = std::make_unique<PIAnaG4StepDivider>();
  merger_ = std::make_unique<PIAnaHitMerger>();
  tcluster_ = std::make_unique<PITCluster>();
}

PIAnalyzer::~PIAnalyzer()
{
  delete hcategories_;
  delete htcluster_;
  delete hdelayed_;
}

void PIAnalyzer::begin()
{
  if (!PIAnaEvtBase::initialized_) {
    PIAnaEvtBase::initialize();
  }

  divider_->step_limit(0.01); // I assume it is in mm
  divider_->g4_step_limit(60); // I assume it is in mm

  merger_->dt_min(10); // I assum it is in ns

  tcluster_->radius(1); // I assume it is in ns

  hcategories_ = new TH1D("hcategories", "#events of each category"
                          ";;Events",
                          20, -0.5, 19.5);
  hcategories_->SetDirectory(PIAnaEvtBase::fout_.get());
  htcluster_ = new TH1D("htcluster", "number of t-clusters"
                        ";Number of t-clusters;Events",
                        10, -0.5, 9.5);
  htcluster_->SetDirectory(PIAnaEvtBase::fout_.get());
  hdelayed_ = new TH1D("hdelayed",
                       "timing of delayed signal"
                       ";Timing of delayed signal t_{delayed} - t_{prompt} [ns]"
                       ";Events / (1 ns)"
                       , 1000, 0, 1000);
  hdelayed_->SetDirectory(PIAnaEvtBase::fout_.get());

  hdecay_true_ = new TH1D("hdecay_true",
                       "timing of decay"
                       ";Timing of decay t_{decay} - t_{stop} [ns]"
                       ";Events / (1 ns)"
                       , 1000, 0, 1000);
  hdecay_true_->SetDirectory(PIAnaEvtBase::fout_.get());

  hdecay_volume_true_ = new TH1D("hdecay_volume_true",
                                 "volume in which pion decays", 40, 0, 400000);
  hdecay_volume_true_->SetDirectory(PIAnaEvtBase::fout_.get());

  // t_ = new TTree("hi_hits", "hti_hits");
  // t_->SetDirectory(PIAnaEvtBase::fout_.get());
  // t_->Branch("hi_nhits", &hi_nhits_, "hi_nhits/s");
  // t_->Branch("hi_xstrip", hi_xstrip_, "hi_xstrip[hi_nhits]/S");
  // t_->Branch("hi_ystrip", hi_ystrip_, "hi_ystrip[hi_nhits]/S");
  // t_->Branch("hi_zlayer", hi_zlayer_, "hi_zlayer[hi_nhits]/S");
  // t_->Branch("hi_edep", hi_edep_, "hi_edep[hi_nhits]/F");
  // t_->Branch("hi_t", hi_t_, "hi_t[hi_nhits]/F");
  // t_->Branch("pi_stop_xstrip", &pi_stop_xstrip_, "pi_stop_xstrip/S");
  // t_->Branch("pi_stop_ystrip", &pi_stop_ystrip_, "pi_stop_ystrip/S");
  // t_->Branch("pi_stop_zlayer", &pi_stop_zlayer_, "pi_stop_zlayer/S");
  // t_->Branch("pi_stop_found", &pi_stop_found_, "pi_stop_found/O");
}

void PIAnalyzer::run()
{
  if (!initialized_) {
    std::cerr << "[ERROR] PIAnalyzer: Not initialized. "
                 "Call PIAnalyzer::begin() before PIAnalyzer::run()\n";
    return;
  }
  for (long long ientry = 0; ientry < chain_->GetEntriesFast(); ++ientry) {

    if (ientry % 1000 == 0) {
      std::cout << "Analyzing " << ientry << " events" << std::endl;
    }

    clear();
    auto itree = chain_->LoadTree(ientry);
    if (itree < 0)
      break;

    auto bytes = chain_->GetEntry(ientry);
    if (bytes <= 0)
      break;

    if (!select_event_id_(info_->GetRun(), info_->GetEvent(),
                          info_->GetEventID())) {
      continue;
    }

    int code = analyze_atar_hits();
    hcategories_->Fill(code);
  }
}

void PIAnalyzer::end()
{
  hcategories_->GetXaxis()->SetBinLabel(PiDAR + 1, "PiDAR");
  hcategories_->GetXaxis()->SetBinLabel(PiDIF + 1, "PiDIF");
  hcategories_->GetXaxis()->SetBinLabel(PiDARNoFirstLayerHit + 1,
                                        "PiDARNoFirstLayerHit");
  hcategories_->GetXaxis()->SetBinLabel(PiDAROffCent+1, "PiDAROffCent");
  hcategories_->GetXaxis()->SetBinLabel(PiDARMergedT + 1, "PiDARMergedT");
  hcategories_->GetXaxis()->SetBinLabel(PiWrongT+1, "PiDARWrongT");
  if (fout_) {
    fout_->Write();
  }
}

void PIAnalyzer::clear()
{
}

int PIAnalyzer::analyze_atar_hits()
{
  // pion must stop
  // KE < 1keV
  // a realistic value will be determined by minimum induced charges
  double pi_stop_time = 0;
  double pi_decay_time = 0;
  double mu_decay_time = -1;
  for (size_t j=0; j<track_->GetEntries(); ++j) {
    auto g4track = dynamic_cast<PIMCTrack*>(track_->At(j));
    // track;
    if (g4track->GetPDGID() == 211) {
      // pion must be within 1cm by 1cm;
      const bool outside = std::abs(g4track->GetPostX().front()) > 1000 ||
        std::abs(g4track->GetPostY().front())>1000;
      if (outside) {
        return PiDAROffCent;
      }

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
        const auto m = m_pion_;
        const auto e2 = px * px + py * py + pz * pz + m * m;
        const auto KE = std::sqrt(e2) - m;
        if (KE < 0.001 && !stopped) {
          stopped =true;
        }
        if (KE < 1E-8) {
          pi_stop_time = t;
          break;
        }
      }

      if (!stopped) {
        if (PIAnaEvtBase::verbose_) {
          Info("PIAnalyzer::analyze_atar_hits", "Pion does not stop. Skip.");
        }
        return PiDIF;
      }
      else {
        if (PIAnaEvtBase::verbose_) {
          Info("PIAnalyzer::analyze_atar_hits", "Pion stops.");
        }
      }
      // if (ts.size() > 1) {
      //   pi_stop_time = ts.at(ts.size() -2);
      // } else {
      //   Warning("PIAnalyzer::analyze_atar_hits",
      //           "Pion only has less than two G4Steps.");
      //   return PiWrongT;
      // }
    }
  }

  for (size_t j = 0; j < decay_->GetEntries(); ++j) {
    // decay
    bool found_pi = false;
    auto decay_info_ = dynamic_cast<PIMCDecay *>(decay_->At(j));
    // assume there is only one pion in each event
    if (decay_info_->GetMotherPDGID() == 211) {
      pi_decay_time = decay_info_->GetTime();

      found_pi = true;

      if (pi_stop_time >= pi_decay_time) {
        std::cout << "Pion stop time " << pi_stop_time << std::endl;
        std::cout << "Pion decay time "<< pi_decay_time << std::endl;
        const std::string msg =
          ::Form("Pion stop time is later than pion decay time. Run:Event:EventID=%d:%d:%d",
                 info_->GetRun(), info_->GetEvent(), info_->GetEventID());
        Warning("PIAnalyzer::analyze_atar_hits", msg.c_str());
        return PiWrongT;
      } else {
        hdecay_true_->Fill(pi_decay_time - pi_stop_time);
      }

      hdecay_volume_true_->Fill(decay_info_->GetVolume());
    }
    if (decay_info_->GetMotherPDGID() == -13) {
      mu_decay_time = decay_info_->GetTime();
    }

    if (!found_pi) {
      const std::string msg =
          ::Form("Not found pion in decay branch. Run:Event:EventID=%d:%d:%d",
                 info_->GetRun(), info_->GetEvent(), info_->GetEventID());
      Warning("PIAnalyzer::analyze_atar_hits", msg.c_str());
      return PiWrongT;
    }
  }

  std::vector<PIAnaHit> rec_hits;
  for (int j=0; j<atar_->GetEntries(); ++j) {
    auto atar_hit = dynamic_cast<PIMCAtar*>(atar_->At(j));
    auto hits = divider_->process_atar_hit(*atar_hit);
    auto merged_hits = merger_->merge(hits);

    std::move(merged_hits.begin(), merged_hits.end(),
              std::back_inserter(rec_hits));
  }

  rec_hits = merger_->merge(rec_hits);

  if (PIAnaEvtBase::verbose_) {
    Info("PIAnalyzer::analyze_atar_hits", "Produced hits in reconstruction level.");
  }

  // prompt cluster
  std::vector<const PIAnaHit*> hit0s;
  for (const auto &hit : rec_hits) {
    if (hit.layer() == 0) {
      hit0s.push_back(&hit);
    }
  }
  if (hit0s.empty()) {
    return PiDARNoFirstLayerHit;
  }
  // pion must be within 1cm by 1cm;
  {
    const bool outside =
        std::find_if(hit0s.begin(), hit0s.end(), [](const PIAnaHit *hit) {
          return std::abs(hit->rec_x()) < 1000 && std::abs(hit->rec_y()) < 1000;
        }) == hit0s.end();
    if (outside)
      return PiDAROffCent;
  }
  std::sort(
      std::begin(hit0s), std::end(hit0s),
      [](const PIAnaHit *h1, const PIAnaHit *h2) { return h1->t() < h2->t(); });

  if (PIAnaEvtBase::verbose_) {
    const std::string msg = ::Form("Found prompt t0: %g", hit0s.front()->t());
    Info("PIAnalyzer::analyze_atar_hits", msg.c_str());
  }

  tcluster_->t0(hit0s.front()->t());

  std::vector<const PIAnaHit *> hitptrs;
  hitptrs.reserve(rec_hits.size());
  // must be auto& hit
  for (const auto &hit : rec_hits) {
    hitptrs.push_back(&hit);
  }
  const auto tclusters =
      tcluster_->cluster_hits(hitptrs.begin(), hitptrs.end());
  if (PIAnaEvtBase::verbose_) {
    std::cout << "[INFO] Number of t-clusters: " << tclusters.size() << "\n";
  }

  htcluster_->Fill(tclusters.size());

  std::vector<double> t0s;
  for (const auto &cluster : tclusters) {
    t0s.push_back(cluster.first->t());
  }
  std::sort(std::begin(t0s), std::end(t0s));
  // first iteration
  double t0_temp = t0s.front();
  int nt0s = 0;
  for (size_t i = 0; i != t0s.size(); ++i) {
    if (t0s.at(i) - t0_temp > 5) {
      t0_temp = t0s.at(i);
      nt0s++;
    }
  }
  if ((nt0s+1) < min_tcluster_) {
    return PiDARMergedT;
  }

  for (const auto &cluster : tclusters) {
    const double dt = cluster.first->t() - tcluster_->t0();
    if (std::abs(dt) < tcluster_->radius())
      continue;
    hdelayed_->Fill(dt);
  }

  return PiDAR;
}

ClassImp(PIAnalyzer)
