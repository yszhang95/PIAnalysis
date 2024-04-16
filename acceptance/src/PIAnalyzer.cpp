#include "PIAnaConst.hpp"
#include "PIAnalyzer.hpp"
#include "PIAnaEvtBase.hpp"
#include "PIAnaHit.hpp"
#include "PIFilterBase.hpp"
#include "PIAnaCluster.hpp"
#include "PIAnaPointCloud.hpp"
#include "PITkFinder.hpp"
#include "PIMCInfo.hh"
#include "PIMCTrack.hh"
#include "PIMCDecay.hh"

#include "TClonesArray.h"
#include "TChain.h"
#include "TError.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TTree.h"
#include "Math/Vector3D.h"
#include "Math/PositionVector3D.h"
#include "Math/Point3Dfwd.h"
#include "Math/Vector3Dfwd.h"
#include <RtypesCore.h>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <memory>
#include <type_traits>

void PIAnalyzer::print_hit(const PIAnaHit& hit)
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
      min_tcluster_(1), pi_dt1_(20), pi_dt2_(5), mu_dt1_(20), mu_dt2_(5),
      merge_hit_dtmin_(20), tcluster_dt_(5), debug_(false), ncluster_(0)
{
  std::fill(std::begin(clusterid_), std::end(clusterid_), 0);
  divider_ = std::make_unique<PIAnaG4StepDivider>();
  merger_ = std::make_unique<PIAnaHitMerger>();
  tcluster_ = std::make_unique<PITCluster>();
  xyzcluster_ = std::make_unique<PIXYZCluster>();
}

PIAnalyzer::~PIAnalyzer()
{
  delete hevtcode_;
  delete hcategories_;
  delete htcluster_;
  delete hdelayed_;
  delete hdecay_true_;
  delete hdecay_volume_true_;
  delete hdecay_xy_diff_;
  delete hdecay_xz_diff_;
  delete hdecay_yz_diff_;
}

void PIAnalyzer::begin()
{
  if (!PIAnaEvtBase::initialized_) {
    PIAnaEvtBase::initialize();
  }

  // divider_->step_limit(0.01); // I assume it is in mm
  divider_->step_limit(0.02); // I assume it is in mm//
  divider_->g4_step_limit(60); // I assume it is in mm

  merger_->dt_min(merge_hit_dtmin_); // I assum it is in ns

  tcluster_->radius(1); // I assume it is in ns

  xyzcluster_->radius(std::sqrt(0.2*0.2*2 + 0.12*0.12)*3); // I assume it is around one pixel

  hevtcode_ = new TH1D("hevtcode",
                       "#events of each code"
                       ";;Events",
                       20, -0.5, 19.5);
  hevtcode_->SetDirectory(PIAnaEvtBase::fout_.get());

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

  hdecay_xz_byz_ = new TH2D("hdecay_xz_byz",
                            "Pion decay point;"
                            "x_{truth} - x_{rec};z_{truth} - z_{rec}",
                            100, -0.1 * 5, 0.1 * 5, 120, -0.06 * 5, 0.06 * 5);
  hdecay_yz_byz_ = new TH2D("hdecay_yz_byz",
                            "Pion decay point;"
                            "y_{truth} - y_{rec};z_{truth} - z_{rec}",
                            100, -0.1 * 5, 0.1 * 5, 120, -0.06 * 5, 0.06 * 5);
  hdecay_xz_bye_ = new TH2D("hdecay_xz_bye",
                            "Pion decay point;"
                            "x_{truth} - x_{rec};z_{truth} - z_{rec}",
                            100, -0.1 * 5, 0.1 * 5, 120, -0.06 * 5, 0.06 * 5);
  hdecay_yz_bye_ = new TH2D("hdecay_yz_bye",
                            "Pion decay point;"
                            "y_{truth} - y_{rec};z_{truth} - z_{rec}",
                            100, -0.1*5, 0.1*5, 120, -0.06*5, 0.06*5);
  hdecay_xz_byz_2hit_ = new TH2D("hdecay_xz_byz_2hit",
                            "Pion decay point;"
                            "x_{rec}^{byz} - x_{rec}^{byz_2hit};z_{rec}^{byz} - z_{rec}^{byz_2hit}",
                            100, -0.1 * 5, 0.1 * 5, 120, -0.06 * 5, 0.06 * 5);
  hdecay_yz_byz_2hit_ = new TH2D("hdecay_yz_byz_2hit",
                            "Pion decay point;"
                            "y_{rec}^{byz} - y_{rec}^{byz_2hit};z_{rec}^{byz} - z_{rec}^{byz_2hit}",
                            100, -0.1 * 5, 0.1 * 5, 120, -0.06 * 5, 0.06 * 5);
  hdecay_xz_bye_2hit_ = new TH2D("hdecay_xz_bye_2hit",
                            "Pion decay point;"
                            "x_{rec}^{bye} - x_{rec}^{bye_2hit};z_{rec}^{bye} - z_{rec}^{bye_2hit}",
                            100, -0.1 * 5, 0.1 * 5, 120, -0.06 * 5, 0.06 * 5);
  hdecay_yz_bye_2hit_ = new TH2D("hdecay_yz_bye_2hit", "Pion decay point;"
                                 "y_{rec}^{bye} - y_{rec}^{bye_2hit};z_{rec}^{bye} - z_{rec}^{bye_2hit}",
                                 100, -0.1*5, 0.1*5, 120, -0.06*5, 0.06*5);

  hdecay_xy_diff_ = new TH2D("hdecay_xy_diff",
                             "Pion decay point;"
                             "x_{truth} - x_{rec};y_{truth} - y_{rec}",
                             100, -0.1*5, 0.1*5, 100, -0.1*5, 0.1*5);
  hdecay_xy_diff_->SetDirectory(PIAnaEvtBase::fout_.get());
  hdecay_xz_diff_ = new TH2D("hdecay_xz_diff",
                             "Pion decay point;"
                             "x_{truth} - x_{rec};z_{truth} - z_{rec}",
                             100, -0.1*5, 0.1*5, 120, -0.06*5, 0.06*5);
  hdecay_xz_diff_->SetDirectory(PIAnaEvtBase::fout_.get());
  hdecay_yz_diff_ = new TH2D("hdecay_yz_diff",
                             "Pion decay point;"
                             "y_{truth} - y_{rec};z_{truth} - z_{rec}",
                             100, -0.1*5, 0.1*5, 120, -0.06*5, 0.06*5);
  hdecay_yz_diff_->SetDirectory(PIAnaEvtBase::fout_.get());


  hdecay_rec_xy_ = new TH2D("hdecay_rec_xy",
                             "Pion decay point in REC level;"
                             "x strip ID;y strip ID",
                             100, -0.5, 99.5, 100, -0.5, 99.5);
  hdecay_rec_xy_->SetDirectory(PIAnaEvtBase::fout_.get());
  hdecay_rec_xz_ = new TH2D("hdecay_rec_xz",
                             "Pion decay point in REC level;"
                             "x strip ID;z layer ID",
                             100, -0.5, 99.5, 48, -0.5, 47.5);
  hdecay_rec_xz_->SetDirectory(PIAnaEvtBase::fout_.get());
  hdecay_rec_yz_ = new TH2D("hdecay_rec_yz",
                             "Pion decay point in REC level;"
                             "y strip ID;z layer ID",
                             100, -0.5, 99.5, 48, -0.5, 47.5);
  hdecay_rec_yz_->SetDirectory(PIAnaEvtBase::fout_.get());


  hdecay_rec_xy_5_ehits_ = new TH2D("hdecay_rec_xy_5_ehits",
                             "Pion decay point in REC level;"
                             "x strip ID;y strip ID",
                             100, -0.5, 99.5, 100, -0.5, 99.5);
  hdecay_rec_xy_5_ehits_->SetDirectory(PIAnaEvtBase::fout_.get());
  hdecay_rec_xz_5_ehits_ = new TH2D("hdecay_rec_xz_5_ehits",
                             "Pion decay point in REC level;"
                             "x strip ID;z layer ID",
                             100, -0.5, 99.5, 48, -0.5, 47.5);
  hdecay_rec_xz_5_ehits_->SetDirectory(PIAnaEvtBase::fout_.get());
  hdecay_rec_yz_5_ehits_ = new TH2D("hdecay_rec_yz_5_ehits",
                             "Pion decay point in REC level;"
                             "y strip ID;z layer ID",
                             100, -0.5, 99.5, 48, -0.5, 47.5);
  hdecay_rec_yz_5_ehits_->SetDirectory(PIAnaEvtBase::fout_.get());

  hdecay_rec_xy_center_ = new TH2D("hdecay_rec_xy_center",
                             "Pion decay point in REC level;"
                             "x strip ID;y strip ID",
                             100, -0.5, 99.5, 100, -0.5, 99.5);
  hdecay_rec_xy_center_->SetDirectory(PIAnaEvtBase::fout_.get());
  hdecay_rec_xz_center_ = new TH2D("hdecay_rec_xz_center",
                             "Pion decay point in REC level;"
                             "x strip ID;z layer ID",
                             100, -0.5, 99.5, 48, -0.5, 47.5);
  hdecay_rec_xz_center_->SetDirectory(PIAnaEvtBase::fout_.get());
  hdecay_rec_yz_center_ = new TH2D("hdecay_rec_yz_center",
                             "Pion decay point in REC level;"
                             "y strip ID;z layer ID",
                             100, -0.5, 99.5, 48, -0.5, 47.5);
  hdecay_rec_yz_center_->SetDirectory(PIAnaEvtBase::fout_.get());


  hdecay_rec_xy_5_ehits_center_ = new TH2D("hdecay_rec_xy_5_ehits_center",
                             "Pion decay point in REC level;"
                             "x strip ID;y strip ID",
                             100, -0.5, 99.5, 100, -0.5, 99.5);
  hdecay_rec_xy_5_ehits_center_->SetDirectory(PIAnaEvtBase::fout_.get());
  hdecay_rec_xz_5_ehits_center_ = new TH2D("hdecay_rec_xz_5_ehits_center",
                             "Pion decay point in REC level;"
                             "x strip ID;z layer ID",
                             100, -0.5, 99.5, 48, -0.5, 47.5);
  hdecay_rec_xz_5_ehits_center_->SetDirectory(PIAnaEvtBase::fout_.get());
  hdecay_rec_yz_5_ehits_center_ = new TH2D("hdecay_rec_yz_5_ehits_center",
                             "Pion decay point in REC level;"
                             "y strip ID;z layer ID",
                             100, -0.5, 99.5, 48, -0.5, 47.5);
  hdecay_rec_yz_5_ehits_center_->SetDirectory(PIAnaEvtBase::fout_.get());

  h_e_rec_thetaphi_ = new TH2D("h_e_rec_thetaphi",
                               "positrion direction in REC level;"
                               "cos(#theta);#phi (#pi rad.);",
                               50, -1, 1, 50, -1, 1);
  h_e_rec_thetaphi_->SetDirectory(PIAnaEvtBase::fout_.get());
  rec_tree_ = new TTree("rec_hits", "reconstructed hits");
  rec_tree_->SetDirectory(PIAnaEvtBase::fout_.get());
  rec_tree_->Branch("ncluster", &ncluster_);
  rec_tree_->Branch("clusterid", clusterid_, "clusterid[ncluster]/I");
  rec_tree_->Branch("x", &this->x_);
  rec_tree_->Branch("y", &this->y_);
  rec_tree_->Branch("z", &this->z_);
  rec_tree_->Branch("t", &this->t_);
  rec_tree_->Branch("de", &this->de_);
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

    std::map<std::string, bool> passed;
    for (const auto& filter : filters_) {
      filter.second->load_event(info_, track_, atar_, decay_);
      passed.insert({filter.first, filter.second->eval_bit()});
    }

    bool ok = true;
    for (const auto& flag : passed) {
      ok = ok && flag.second;
      const auto evtcode = PIAna::EvtCode(filters_.at(flag.first)->get_code());
      if (flag.second) {
        hevtcode_->Fill(static_cast<int>(evtcode));
      }

      if (PIAnaEvtBase::verbose_) {
        switch (evtcode) {
        case PIAna::EvtCode::PiDAR: {
          if (!flag.second)
            Info("PIAnalyzer::analyze_atar_hits", "Pion does not stop. Skip.");
          break;
        }
        default:
          break;
        }
      }
    }

    if (!ok) continue;

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
  hcategories_->GetXaxis()->SetBinLabel(PiOffATAR+1, "PiOffATAR");
  hcategories_->GetXaxis()->SetBinLabel(PiDARMergedT + 1, "PiDARMergedT");
  hcategories_->GetXaxis()->SetBinLabel(PiWrongT+1, "PiDARWrongT");
  if (fout_) {
    fout_->Write();
  }
}

void PIAnalyzer::clear()
{
  std::fill(std::begin(clusterid_), std::end(clusterid_), 0);
  pi_decay_x_ = -1E10;
  pi_decay_y_ = -1E10;
  pi_decay_z_ = -1E10;

  pi_decay_x_true_ = -1E10;
  pi_decay_y_true_ = -1E10;
  pi_decay_z_true_ = -1E10;

}

int PIAnalyzer::analyze_atar_hits()
{
  // pion must stop
  // KE < 1keV
  // a realistic value will be determined by minimum induced charges
  double pi_stop_time = 0;
  double pi_decay_time = 0;
  double mu_decay_time = -1;
  double pi_decay_KE = 0;
  int pi_decay_volume = 0;

  if (debug_) {
    std::ofstream ofile_atar("atar.txt");
    ofile_atar << "PDGID TIME POSTX POSTY POSTZ\n";
    for (int j=0; j<atar_->GetEntries(); ++j) {
      auto atar_hit = dynamic_cast<PIMCAtar*>(atar_->At(j));
      ofile_atar << atar_hit->GetPDGID() << " "
                 << atar_hit->GetTime() << " "
                 << atar_hit->GetX1() << " "
                 << atar_hit->GetY1() << " "
                 << atar_hit->GetZ1() << " "
                 << "\n";
    }
    std::ofstream ofile("steps.txt");
    ofile << "PDGID POSTT POSTX POSTY POSTZ POSTPX POSTPY POSTPZ EDEP PROCESS VOLUME\n";
    for (size_t j=0; j<track_->GetEntries(); ++j) {
      auto g4track = dynamic_cast<PIMCTrack *>(track_->At(j));

      uint64_t nsteps = g4track->GetPostMomX().size();
      for (uint64_t istep = 0; istep < nsteps; ++istep) {
        // pdgid, t, x, y, z, px, py, pz, Edep, process, volume
        ofile << g4track->GetPDGID() << " "
              << g4track->GetPostTime().at(istep)
              << " " << g4track->GetPostX().at(istep) << " "
              << g4track->GetPostY().at(istep) << " "
              << g4track->GetPostZ().at(istep)
              << " "
              << g4track->GetPostMomX().at(istep) << " "
              << g4track->GetPostMomY().at(istep) << " "
              << g4track->GetPostMomZ().at(istep) << " "
              << g4track->GetEdep().at(istep) << " "
              << g4track->GetProcessID().at(istep) << " "
              << g4track->GetVolume().at(istep)
              << "\n";
      }
    }

    std::ofstream ofile_rechits("rec_hits.txt");
    ofile_rechits << "PDGID PRET TRUEPOSTX TRUEPOSTY TRUEPOSTZ XSTRIP YSTRIP ZLAYER\n";
    std::vector<PIAnaHit> rec_hits;
    for (int j=0; j<atar_->GetEntries(); ++j) {
      auto atar_hit = dynamic_cast<PIMCAtar*>(atar_->At(j));
      auto hits = divider_->process_atar_hit(*atar_hit);
      auto merged_hits = merger_->merge(hits);

      std::move(merged_hits.begin(), merged_hits.end(),
                std::back_inserter(rec_hits));
    }

    rec_hits = merger_->merge(rec_hits);

    std::sort(rec_hits.begin(), rec_hits.end(),
              [](const PIAnaHit& hit1, const PIAnaHit& hit2)
              {return hit1.t() < hit2.t();});
    for (const auto hit: rec_hits) {
      ofile_rechits
        << hit.pdgid()  << " "
        << hit.t() << " "
        << hit.x() << " "
        << hit.y() << " "
        << hit.z() << " "
        << hit.xstrip() << " "
        << hit.ystrip() << " "
        << hit.layer()
        << "\n";
    }
  }


  for (size_t j=0; j<track_->GetEntries(); ++j) {
    auto g4track = dynamic_cast<PIMCTrack *>(track_->At(j));

    // track;
    if (g4track->GetPDGID() == 211) {
      // pion must be within 2cm by 2cm;
      const bool outside = std::abs(g4track->GetPostX().front()) > 1000 ||
        std::abs(g4track->GetPostY().front())>1000;
      if (outside) {
        return PiOffATAR;
      }

      // assume they are sorted by time
      const auto& ts = g4track->GetPostTime();
      const auto &pxs = g4track->GetPostMomX();
      const auto &pys = g4track->GetPostMomY();
      const auto &pzs = g4track->GetPostMomZ();

      const auto &volumes = g4track->GetVolume();

      auto in_sens_det = volumes.end()
        != std::find_if(volumes.begin(), volumes.end(),
                        [](const int vol) { return vol >= 180000 && vol <185000;} );
      if (!in_sens_det) {
        // std::cout << "Pion not in senstive detector in Run:Event:EventID = "
        //           << info_->GetRun() << ":" << info_->GetEvent() << ":"
        //           << info_->GetEventID() << ".\n";
        // for (const auto vol : volumes) {
        //   std::cout << vol << "\t";
        // }
        // std::cout << std::endl;
        return PiOffATAR;
      }

      pi_decay_x_ = g4track->GetPostX().back();
      pi_decay_y_ = g4track->GetPostY().back();
      pi_decay_z_ = g4track->GetPostZ().back();

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
        if (KE < 1E-8) {
          pi_stop_time = t;
          break;
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

  bool found_pi = false;
  for (size_t j = 0; j < decay_->GetEntries(); ++j) {
    // decay
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
      pi_decay_KE = decay_info_->GetMotherEnergy();
      pi_decay_volume = decay_info_->GetVolume();
      auto position = decay_info_->GetPosition();
      pi_decay_x_ = position.X();
      pi_decay_y_ = position.Y();
      pi_decay_z_ = position.Z();
    }
    if (decay_info_->GetMotherPDGID() == -13) {
      mu_decay_time = decay_info_->GetTime();
    }
  }
  if (!found_pi) {
    const std::string msg =
      ::Form("Not found pion in decay branch. Run:Event:EventID=%d:%d:%d",
             info_->GetRun(), info_->GetEvent(), info_->GetEventID());
    Warning("PIAnalyzer::analyze_atar_hits", msg.c_str());
    return PiWrongT;
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

  // find decay xyz for pi in reco
  {
    std::vector<const PIAnaHit *> pis;
    for (const auto &hit : rec_hits) {
      bool found = false;
      for (const auto id : hit.pdgids()) {
        if (id == 211) {
          found = true;
          break;
        }
      }
      if (found) pis.push_back(&hit);
    }
    std::sort(pis.begin(), pis.end(),
              [](const PIAnaHit *h1, const PIAnaHit *h2) {
                return h1->t() < h2->t();
              });
    if (pis.empty()) {
      std::cout << "Not found pion hit in run:event:eventid = "<< info_->GetRun() << ":" << info_->GetEvent() << ":" << info_->GetEventID() << ".\n";
      std::cout << "decay pion position (x,y,z) = (" << pi_decay_x_ << "," << pi_decay_y_ << "," << pi_decay_z_ << ") with kinetic energy " << pi_decay_KE << " MeV in volume " << pi_decay_volume << ".\n";
      for (const auto &hit : rec_hits) {
        std::cout << hit.pdgid()  << "(" << hit.pdgids().size() << ")"<< "\t";
      }
      std::cout << "\n";
    } else {
      pi_decay_x_true_ = pis.back()->rec_x();
      pi_decay_y_true_ = pis.back()->rec_y();
      pi_decay_z_true_ = pis.back()->rec_z();
    }
  }

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
  // // pion must be within 2cm by 2cm;
  // {
  //   const bool outside =
  //       std::find_if(hit0s.begin(), hit0s.end(), [](const PIAnaHit *hit) {
  //         return std::abs(hit->rec_x()) < 1000 && std::abs(hit->rec_y()) < 1000;
  //       }) == hit0s.end();
  //   if (outside)
  //     return PiOffATAR;
  // }
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
    if (t0s.at(i) - t0_temp > tcluster_dt_) {
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

  // find out pi vertex from earliest cluster
  const PIAnaHit* hitbyz, *hitbye;
  {
    const PIAnaHit *ptr = nullptr;
    double t0 = 10000000;
    for (const auto &cluster : tclusters) {
      if (cluster.first->t() < t0) {
        ptr = cluster.first;
        t0 = cluster.first->t();
      }
    }
    std::vector<const PIAnaHit *> hits_copy = tclusters.at(ptr);
    if (hits_copy.empty()) {
      std::cerr << "Why the collection is empty for prompt timing?\n";
      return -10;
    }
    // by z
    std::sort(hits_copy.begin(), hits_copy.end(),
              [](const PIAnaHit *h1, const PIAnaHit *h2) {
                return h1->rec_z() > h2->rec_z();
              });
    // for (const auto hit : hits_copy) {
    //   std::cout << hit->rec_z() << "\t";
    // }
    // std::cout << "\n";
    hitbyz = hits_copy.front();
    // std::cout << "hitbyz " << hitbyz->rec_z() << " true z " << pi_decay_z_ << "\n";
    hdecay_xz_byz_->Fill(pi_decay_x_ - hitbyz->rec_x(),
                         pi_decay_z_ - hitbyz->rec_z());
    hdecay_yz_byz_->Fill(pi_decay_y_ - hitbyz->rec_y(),
                         pi_decay_z_ - hitbyz->rec_z());

    // by e
    std::sort(hits_copy.begin(), hits_copy.end(),
              [](const PIAnaHit *h1, const PIAnaHit *h2) {
                return h1->edep() > h2->edep();
              });
    hitbye = hits_copy.front();
    hdecay_xz_bye_->Fill(pi_decay_x_ - hitbye->rec_x(),
                         pi_decay_z_ - hitbye->rec_z());
    hdecay_yz_bye_->Fill(pi_decay_y_ - hitbye->rec_y(),
                         pi_decay_z_ - hitbye->rec_z());
  }

  hdecay_rec_xy_->Fill(hitbyz->xstrip(), hitbyz->ystrip());
  hdecay_rec_xz_->Fill(hitbyz->xstrip(), hitbyz->layer());
  hdecay_rec_yz_->Fill(hitbyz->ystrip(), hitbyz->layer());

  // xyz cluster
  std::map<const PIAnaHit *, std::vector<std::vector<const PIAnaHit *>>>
      txyz_connected_hits;
  for (const auto &tcluster : tclusters) {
    txyz_connected_hits.insert({tcluster.first, {}});
    auto clusters = xyzcluster_->cluster_hits(tcluster.second.begin(),
                                              tcluster.second.end());
    for (const auto& hits : clusters) {
      txyz_connected_hits.at(tcluster.first).push_back(hits.second);
    }
  }

  // find decay point
  std::vector<std::pair<const PIAnaHit*, const PIAnaHit*> > decay_points;
  for (auto it = txyz_connected_hits.begin(); it != txyz_connected_hits.end();
       ++it) {
    for (auto it2 = std::next(it); it2 != txyz_connected_hits.end(); ++it2) {
      const auto dt = std::abs(it->first->t() - it2->first->t());
      // std::cout << dt << std::endl;
      // hits not merged;
      if (dt > pi_dt1_ || dt > mu_dt1_) {
        for (const auto &hits : it->second) {
          for (const auto &hits2 : it2->second) {
            auto points = decay_point(hits, hits2);
            std::move(points.begin(), points.end(), std::back_inserter(decay_points));
          }
        }
      }
      // hits merged
      if (dt > pi_dt2_ || dt > mu_dt2_) {
      }
    }
  }
  // refine later
  // the first one is pion, may be wrong

  if (!decay_points.empty()) {
    // choose the largest z as decay point
    std::sort(std::begin(decay_points), std::end(decay_points),
              [](const std::pair<const PIAnaHit *, const PIAnaHit *> p1,
                 const std::pair<const PIAnaHit *, const PIAnaHit *> p2) {
                return p1.first->rec_z() < p2.first->rec_z();
              });

    const double x = decay_points.back().first->rec_x();
    const double y = decay_points.back().first->rec_y();
    const double z = decay_points.back().first->rec_z();
    hdecay_xy_diff_->Fill(pi_decay_x_ - x, pi_decay_y_ - y);
    hdecay_xz_diff_->Fill(pi_decay_x_ - x, pi_decay_z_ - z);
    hdecay_yz_diff_->Fill(pi_decay_y_ - y, pi_decay_z_ - z);

    hdecay_xz_byz_2hit_->Fill(hitbyz->rec_x() - x, hitbyz->rec_z() - z);
    hdecay_yz_byz_2hit_->Fill(hitbyz->rec_y() - y, hitbyz->rec_z() - z);
    {
      const bool xoff = std::abs(pi_decay_x_ - x) > 0.1;
      const bool yoff = std::abs(pi_decay_y_ - y) > 0.1;
      if (xoff || yoff) {
        std::cout << "x or y difference > 0.1 in Run:Event:EventID = "
                  << info_->GetRun() << ":" << info_->GetEvent() << ":"
                  << info_->GetEventID() << "\n";
        auto print_decay = [x, y, z, this](const PIAnaHit *hit) {
          std::cout << "[DEBUG] " << "At reco decay location: pdgid = "
                    << hit->pdgid()
                    << " at t = " << hit->t() << "\n";
          std::cout << "[DEBUG] rec hit x " << x << ", true x at rec hit " << hit->x()
                    << ", true decay x " << pi_decay_x_ << "\n";
          std::cout << "[DEBUG] rec hit y " << y << ", true y at rec hit "
                    << hit->y() << ", true decay y " << pi_decay_y_ << "\n";
          std::cout << "[DEBUG] rec hit z " << z << ", true z at rec hit "
                    << hit->z() << " true z " << pi_decay_z_ << "\n";
        };
        std::cout << "[DEBUG] The first hit at reco decay location\n";
        print_decay(decay_points.back().first);
        std::cout << "[DEBUG] The second hit at reco decay location\n";
        print_decay(decay_points.back().second);
      }
    }

    // by e
    std::sort(std::begin(decay_points), std::end(decay_points),
              [](const std::pair<const PIAnaHit *, const PIAnaHit *> p1,
                 const std::pair<const PIAnaHit *, const PIAnaHit *> p2) {
                return p1.first->edep() < p2.first->edep();
              });
    hdecay_xz_bye_2hit_->Fill(hitbye->rec_x() - decay_points.back().first->rec_x(),
                              hitbye->rec_z() - decay_points.back().first->rec_z());
    hdecay_yz_bye_2hit_->Fill(hitbye->rec_y() - decay_points.back().first->rec_y(),
                              hitbye->rec_z() - decay_points.back().first->rec_z());
  } else {
    std::cout
        << "[DEBUG] Not found any pixel with two hits at Run:Event:EventID = "
        << info_->GetRun() << ":" << info_->GetEvent() << ":" << info_->GetEventID() << "\n";
  }

  // hard code // cannot handle all cases
  // use the last collection in timing
  // I asumme there is only one collection for each timing
  const PIAnaHit *lastptr = nullptr;
  // for (const auto &hit : txyz_connected_hits)
  for (const auto& hit : tclusters)
    {
      if (lastptr) {
        if (lastptr->t() < hit.first->t()) {
          lastptr = hit.first;
        }
      } else {
        lastptr = hit.first;
      }
  }

  // if (txyz_connected_hits.at(lastptr).size() != 1) {
  //   char msg[200];
  //   sprintf(msg,
  //           "%zu connected xyz components. "
  //           "Run:Event:EventID=%d:%d:%d", txyz_connected_hits.at(lastptr).size(),
  //           info_->GetRun(), info_->GetEvent(), info_->GetEventID());
  //   Error("PIAnalyzer::analyze_atar_hits", msg);
  //   for (const auto &cluster : txyz_connected_hits.at(lastptr)) {
  //     std::cout << "Cluster t0 " << lastptr->t() << "\n";
  //     for (const auto &hit : cluster) {
  //       std::cout << hit->t() << "\t" << hit->xstrip() << "\t" << hit->ystrip() << "\t" << hit->layer() << "\t" << hit->pdgid()<<"\n";
  //     }
  //   }
  // }

  PIAnaPointCloud::Point pivertex{hitbyz->rec_x(), hitbyz->rec_y(), hitbyz->rec_z()};

  // const auto posi_dir =
  //     positron_direction(txyz_connected_hits.at(lastptr).front(), pivertex);
  const auto posi_dir = positron_direction(tclusters.at(lastptr), pivertex);
  if (posi_dir.first) {
    hdecay_rec_xy_5_ehits_->Fill(hitbyz->xstrip(), hitbyz->ystrip());
    hdecay_rec_xz_5_ehits_->Fill(hitbyz->xstrip(), hitbyz->layer());
    hdecay_rec_yz_5_ehits_->Fill(hitbyz->ystrip(), hitbyz->layer());

    if (hitbyz->xstrip() < 90 && hitbyz->xstrip() >= 10 &&
        hitbyz->ystrip() < 90 && hitbyz->ystrip() >= 10) {
      h_e_rec_thetaphi_->Fill(std::cos(posi_dir.second.Theta()),
                              posi_dir.second.Phi() / TMath::Pi());
      hdecay_rec_xy_5_ehits_center_->Fill(hitbyz->xstrip(), hitbyz->ystrip());
      hdecay_rec_xz_5_ehits_center_->Fill(hitbyz->xstrip(), hitbyz->layer());
      hdecay_rec_yz_5_ehits_center_->Fill(hitbyz->ystrip(), hitbyz->layer());
    }
  }
  if (hitbyz->xstrip() < 90 && hitbyz->xstrip() >= 10 &&
      hitbyz->ystrip() < 90 && hitbyz->ystrip() >= 10) {
    if (!posi_dir.first) {
      std::cout << "Event with pion decay in center has no more than 5 "
                   "positron hits: "
                << "Run:Event:EventID = " << info_->GetRun() << ":"
                << info_->GetEvent() << ":" << info_->GetEventID() << "\n";
      std::cout << "The pion decay vertex in truth is at (x, y, z) = "
                << "(" << pi_decay_x_ << ", " << pi_decay_y_ << ", " << pi_decay_z_
                << ").\n";
    }
  }
  // if (posi_dir.first) {
  // } else {
  //   Warning("PIAnalyzer::analyze_atar_hits", "There must be at least 5 hits for positron");
  // }

  int index = 0;
  for (auto it = txyz_connected_hits.begin(); it != txyz_connected_hits.end();
       ++it) {
    int code = (index++) * 100;
    for (const auto &hits : it->second) {
      code++;
      this->x_.push_back({});
      this->y_.push_back({});
      this->z_.push_back({});
      this->t_.push_back({});
      this->de_.push_back({});
      for (const auto hit : hits) {
        x_.back().push_back(hit->rec_x());
        y_.back().push_back(hit->rec_y());
        z_.back().push_back(hit->rec_z());
        t_.back().push_back(hit->t());
        de_.back().push_back(hit->edep());
      }
      clusterid_[this->ncluster_++] = code;
    }
  }
  rec_tree_->Fill();

  return PiDAR;
}

std::vector<std::pair<const PIAnaHit *, const PIAnaHit* > >
PIAnalyzer::decay_point(const std::vector<const PIAnaHit*>& hits1, const std::vector<const PIAnaHit*>& hits2)
{
  std::vector<std::pair<const PIAnaHit *, const PIAnaHit* > > results;
  auto xyzgraph = PIAnaGraph(3);
  for (const auto& hit : hits2) {
    xyzgraph.AddPoint(hit);
  }
  for (const auto& hit : hits1) {
    PIAnaPointCloud::Point p{hit->rec_x(), hit->rec_y(), hit->rec_z()};
    auto idxs = xyzgraph.connected_components(p, 0.5);
    for (const auto &idx : idxs) {
      const auto hit2 = xyzgraph.get_hit(idx);
      if (hit->xstrip() == hit2->xstrip() &&
          hit->ystrip() == hit2->ystrip() &&
          hit->layer() == hit2->layer())
        {
        std::pair<const PIAnaHit *, const PIAnaHit *> result
          = hit->t() < hit2->t() ? std::make_pair(hit, hit2) :
          std::make_pair(hit2, hit);
        results.push_back(result);
        }
    }
  }
  return results;
}

std::pair<bool, ROOT::Math::Polar3DVector>
PIAnalyzer::positron_direction(const std::vector<const PIAnaHit *> hits,
                   const PIAnaPointCloud::Point pivertex)
{
  if (hits.size() < 5) {
    Warning("PIAnalyzer::position_direction", "The size of input positron hit collection is less than 5.");
      return {false, {}};
  }

  auto xyzgraph = PIAnaGraph(3);
  for (const auto& hit : hits) {
    xyzgraph.AddPoint(hit);
  }
  const auto indices = xyzgraph.connected_components(pivertex, 5);
  if (indices.size() < 5) {
    Warning("PIAnalyzer::position_direction", "The size of serach by graph is less than 5.");
      return {false, {}};
  }

  auto pca = PIAna::PITkPCA(3);
  for (unsigned int i = 0; i != 5; ++i) {
    const auto hit = xyzgraph.get_hit(indices.at(i));
    double v[3] = {hit->rec_x(), hit->rec_y(), hit->rec_z()};
    pca.add_data(v);
  }
  pca.fit();

  // reference point
  const auto piref = ROOT::Math::XYZPoint(pivertex.x, pivertex.y, pivertex.z);
  const auto iter1_dire = pca.get_direction();

  // the second iteration
  auto extremes = [&piref, &iter1_dire](const PIAnaHit *h1, const PIAnaHit *h2) -> bool {
    const auto p1 = ROOT::Math::XYZPoint(h1->rec_x(), h1->rec_y(), h1->rec_z());
    const auto p2 = ROOT::Math::XYZPoint(h2->rec_x(), h2->rec_y(), h2->rec_z());
    const auto d1 = p1 - piref;
    const auto d2 = p1 - piref;
    return d1.Dot(iter1_dire) < d1.Dot(iter1_dire);
  };
  const auto p_extremes = std::minmax_element(hits.begin(), hits.end(), extremes);
  const PIAnaHit *const ptr1 = *p_extremes.first;
  const PIAnaHit* const ptr2 = *p_extremes.second;
  const auto p1 =
      ROOT::Math::XYZPoint(ptr1->rec_x(), ptr1->rec_y(), ptr1->rec_z());
  const auto p2 =
      ROOT::Math::XYZPoint(ptr2->rec_x(), ptr2->rec_y(), ptr2->rec_z());
  const auto eposition = (p1-piref).Mag2() < (p2-piref).Mag2() ?  p1 : p2;

  const PIAnaPointCloud::Point evertex{eposition.X(), eposition.Y(),
                                       eposition.Z()};
  pca.clear_data();

  const auto indices2 = xyzgraph.connected_components(evertex, 5);
  for (unsigned int i=0; i != indices2.size(); ++i) {
    const auto hit = xyzgraph.get_hit(indices2.at(i));
    double v[3] = {hit->rec_x(), hit->rec_y(), hit->rec_z()};
    pca.add_data(v);
  }
  pca.fit();

  const auto iter2_dire = pca.get_direction();
  return {true, iter2_dire};
}
