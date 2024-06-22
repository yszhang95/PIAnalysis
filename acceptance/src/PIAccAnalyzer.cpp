/*********************************************************************
*                                                                    *
* 2024-04-09 15:54:16                                                *
* acceptance/src/PIAccAnalyzer.cpp                                   *
*                                                                    *
*********************************************************************/

#include "PIMCInfo.hh"
#include "TError.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2D.h"
#include "Math/Vector3D.h"
#include "Math/Point3D.h"
#include "Math/GenVector/VectorUtil.h"
#include "TMath.h"

#include "PIJobManager.hpp"
#include "PIEventData.hpp"
#include "PIAccAnalyzer.hpp"
#include "PIAnaHit.hpp"
#include "PIPointWrapper.hpp"

PIAna::PIAccAnalyzer::PIAccAnalyzer(const std::string &name)
    : PIEventAnalyzer(name), t_(0), h_pistop_rec_xy_(0),
      h_pistop_rec_xy_5hits_(0), h_e_rec_angle_(0),
      h_e_angle_diff_(0),
      h_e_angle_diff_p_(0)
{
  // fill out what is necessary
}

PIAna::PIAccAnalyzer::~PIAccAnalyzer()
{
  // fill out what is necessary
}

void PIAna::PIAccAnalyzer::Begin()
{
  // always call Begin() of base class first to ensure things are initialized.
  PIEventAnalyzer::Begin();
  // always check if manager is available.
  if (!PIEventAction::mgr_) {
    Fatal("PIAna::PIAccAnalyzer", "PIJobManager is not set.");
  }
  if (PIEventAction::mgr_->out_file()) {
    PIEventAnalyzer::outputfile_ = PIEventAction::mgr_->out_file();
  } else {
    Fatal("PIAna::PIAccAnalyzer::Begin()", "Output file is not properly initialized.");
  }
  // fill out what is necessary from here.
  t_ = new TTree("topo", "topo");
  t_->Branch("pi_x", &pi_x_);
  t_->Branch("pi_y", &pi_y_);
  t_->Branch("pi_z", &pi_z_);
  t_->Branch("pi_rec_x", &pi_rec_x_);
  t_->Branch("pi_rec_y", &pi_rec_y_);
  t_->Branch("pi_rec_z", &pi_rec_z_);
  t_->Branch("e_x", &e_x_);
  t_->Branch("e_y", &e_y_);
  t_->Branch("e_z", &e_z_);
  t_->Branch("e_pt", &e_pt_);
  t_->Branch("e_theta", &e_theta_);
  t_->Branch("e_phi", &e_phi_);
  t_->Branch("e_rec_x", &e_rec_x_);
  t_->Branch("e_rec_y", &e_rec_y_);
  t_->Branch("e_rec_z", &e_rec_z_);
  t_->Branch("e_rec_theta", &e_rec_theta_);
  t_->Branch("e_rec_phi", &e_rec_phi_);
  t_->Branch("e_rec_nhits", &e_rec_nhits_, "e_rec_nhits/b");
  t_->Branch("e_rec_xs", e_rec_xs_, "e_rec_xs[e_rec_nhits]/F");
  t_->Branch("e_rec_ys", e_rec_ys_, "e_rec_ys[e_rec_nhits]/F");
  t_->Branch("e_rec_zs", e_rec_zs_, "e_rec_zs[e_rec_nhits]/F");
  t_->SetDirectory(PIEventAnalyzer::outputfile_);

  h_pistop_rec_xy_ = new TH2D(
      "h_pistop_rec_xy", "#pi^{+} stop position in reco level;x (mm); y (mm);",
      100, -10, 10, 100, -10, 10);
  h_pistop_rec_xy_->SetDirectory(PIEventAnalyzer::outputfile_);
  h_pistop_rec_xy_5hits_ =
      new TH2D("h_pistop_rec_xy_5hits",
               "#pi^{+} stop position in reco level && 5 e hits;x (mm); y (mm);",
               100, -10, 10, 100, -10, 10);
  h_pistop_rec_xy_5hits_->SetDirectory(PIEventAnalyzer::outputfile_);

  h_e_rec_angle_ =
      new TH2D("h_e_rec_angle",
               "e^{+} outgoing directions in reco;#phi_{e} (#pi rad.);cos(#theta_{e});",
               50, -1, 1, 50, -1, 1);
  h_e_rec_angle_->SetDirectory(PIEventAnalyzer::outputfile_);

  h_e_truemom_angle_ =
      new TH2D("h_e_truemom_angle",
               "True #vec{P}_{e^{+}} direction;#phi_{e} (#pi rad.);cos(#theta_{e});",
               50, -1, 1, 50, -1, 1);
  h_e_truemom_angle_->SetDirectory(PIEventAnalyzer::outputfile_);

  h_e_angle_diff_ =
      new TH2D("h_e_angle_diff",
               "Residuals of e^{+} direction in solid angles;#phi_{rec} - "
               "#phi_{mom} (#pi rad);cos(#theta_{rec}) - cos(#theta_{mom});",
               50, -1, 1, 50, -2, 2);
  h_e_angle_diff_->SetDirectory(PIEventAnalyzer::outputfile_);

  h_e_angle_diff_p_ = new TH2D(
      "h_e_angle_diff_p",
      "Residuals of e^{+} direction in solid angles (dx or dy > 0.2 *2 mm);"
      "#phi_{rec} - #phi_{mom} (#pi rad);cos(#theta_{rec}) - cos(#theta_{mom});",
      50, -1, 1, 50, -2, 2);
  h_e_angle_diff_p_->SetDirectory(PIEventAnalyzer::outputfile_);

  // give a summary of parameters at the end.
  if (PIEventAction::verbose_) {
    report();
  }
}

void PIAna::PIAccAnalyzer::DoAction(PIEventData& event)
{
  // Do not modify this function.
  PIEventAnalyzer::DoAction(event);
}

void PIAna::PIAccAnalyzer::End()
{
  // fill out what is necessary from here.

  // always call End() of base class at the end.
  PIEventAnalyzer::End();
}

void PIAna::PIAccAnalyzer::analyze(const PIEventData& event)
{
  // this method is called in PIEventAnalyzer::DoAction().
  // fill out what is necessary here.
  clear();
  const auto &eventinfo = event.Get<const PIMCInfo*>("info");
  const auto &flag = event.Get<bool>(topoflag_name_);

  const auto &pivertex = event.Get<const ROOT::Math::XYZPoint>(pivertex_name_);
  const auto &true_pivertex =
      event.Get<const XYZPointWrapper>(true_pivertex_name_);
  const auto &estart = event.Get<const ROOT::Math::XYZPoint>(estart_name_);
  const auto &ehits = event.Get<const std::vector<ROOT::Math::XYZPoint > >(ehits_name_);
  const auto &true_estart = event.Get<const XYZPointWrapper>(true_estart_name_);
  const auto &edirection =
      event.Get<const ROOT::Math::Polar3DVector>(edirection_name_);
  const auto &etruemom = event.Get<const ROOT::Math::XYZVector>(etruemom_name_);

  if (true_pivertex.null() || true_estart.null()) return;
  pi_x_ = true_pivertex.point().X();
  pi_y_ = true_pivertex.point().Y();
  pi_z_ = true_pivertex.point().Z();
  pi_rec_x_ = pivertex.X();
  pi_rec_y_ = pivertex.Y();
  pi_rec_z_ = pivertex.Z();
  e_x_ = true_estart.point().X();
  e_y_ = true_estart.point().Y();
  e_z_ = true_estart.point().Z();
  e_pt_ = etruemom.Rho();
  e_theta_ = etruemom.Theta();
  e_phi_ = etruemom.Phi();
  e_rec_x_ = estart.X();
  e_rec_y_ = estart.Y();
  e_rec_z_ = estart.Z();
  e_rec_theta_ = edirection.Theta();
  e_rec_phi_ = edirection.Phi();

  if (!ehits.empty() && ehits.size() != 5) {
    Fatal("PIAna::PIAnaAnalyzer",
          ::Form("Number of reconstructed hits from %s is not 5",
                 ehits_name_.c_str()));
  }
  e_rec_nhits_ = ehits.size();

  for (size_t i = 0; i != 5 && !ehits.empty(); ++i) {
    e_rec_xs_[i] = ehits.at(i).X();
    e_rec_ys_[i] = ehits.at(i).Y();
    e_rec_zs_[i] = ehits.at(i).Z();
  }

  // hard code
  const bool not_valid_pi =
      pi_rec_x_ < -1E8 && pi_rec_y_ < -1E8 && pi_rec_z_ < -1E8;
  if (not_valid_pi)
    return;
  t_->Fill();
  h_pistop_rec_xy_->Fill(pi_rec_x_, pi_rec_y_);
  if (flag) {
    h_pistop_rec_xy_5hits_->Fill(pi_rec_x_, pi_rec_y_);
    const auto xstrip = PIAnaHit::find_strip(pi_rec_x_);
    const auto ystrip = PIAnaHit::find_strip(pi_rec_y_);
    if (xstrip.first < 90 && xstrip.first >= 10 && ystrip.first < 90 &&
        ystrip.first >= 10) {
      h_e_rec_angle_->Fill(e_rec_phi_ / TMath::Pi(), TMath::Cos(e_rec_theta_));
      h_e_truemom_angle_->Fill(etruemom.Phi() / TMath::Pi(),
                               TMath::Cos(etruemom.Theta()));
      const double delta_phi = ROOT::Math::VectorUtil::Phi_mpi_pi(e_rec_phi_ - etruemom.Phi());
      const double delta_costheta = TMath::Cos(e_rec_theta_) -
          TMath::Cos(etruemom.Theta());

      h_e_angle_diff_->Fill(delta_phi / TMath::Pi(), delta_costheta);

      const double dxordy = 0.21;
      const double x1 = estart.X();
      const double y1 = estart.Y();
      for (const auto &p : ehits) {
        const double x2 = p.X();
        const double y2 = p.Y();
        if (std::abs(x2 - x1) > dxordy || std::abs(y2 - y1) > dxordy) {
          h_e_angle_diff_p_->Fill(delta_phi / TMath::Pi(), delta_costheta);
          break;
        }
      }
      if (PIEventAction::verbose_) {
        if (std::abs(delta_costheta) > 0.5) {
          Error("PIAna::PIAccAnalyzer",
                ::Form("Delta cosTheta %f > 0.5 at run:event:eventid = %d:%d:%d", delta_costheta,
                      eventinfo->GetRun(), eventinfo->GetEvent(), eventinfo->GetEventID()));
        }
      }
    }
  }
}


void PIAna::PIAccAnalyzer::report()
{
  // fill out what is necessary from here.
}

void PIAna::PIAccAnalyzer::clear()
{
    pi_x_ = -1E9;
    pi_y_ = -1E9;
    pi_z_ = -1E9;
    pi_rec_x_ = -1E9;
    pi_rec_y_ = -1E9;
    pi_rec_z_ = -1E9;
    e_x_ = -1E9;
    e_y_ = -1E9;
    e_z_ = -1E9;
    e_pt_ = -1E9;
    e_theta_ = -1E9;
    e_phi_ = -1E9;
    e_rec_x_ = -1E9;
    e_rec_y_ = -1E9;
    e_rec_z_ = -1E9;
    e_rec_theta_ = -1E9;
    e_rec_phi_ = -1E9;
    e_rec_nhits_ = 0;
    std::fill_n(e_rec_xs_, 5, -1E9);
    std::fill_n(e_rec_ys_, 5, -1E9);
    std::fill_n(e_rec_zs_, 5, -1E9);
}
