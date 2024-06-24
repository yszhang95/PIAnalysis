/*********************************************************************
*                                                                    *
* 2024-06-24 08:18:02                                                *
* acceptance/src/PIMichelEnergyAnalyzer.cpp                          *
*                                                                    *
*********************************************************************/

#include "TClonesArray.h"
#include "TError.h"
#include "TFile.h"
#include "TH1D.h"

#include "PIMCInfo.hh"
#include "PIMCDecay.hh"

#include "PIJobManager.hpp"
#include "PIEventData.hpp"
#include "PIAnaConst.hpp"
#include "PIMichelEnergyAnalyzer.hpp"

PIAna::PIMichelEnergyAnalyzer::PIMichelEnergyAnalyzer(const std::string& name)
  : PIEventAnalyzer(name), ke_threshold_(1E-3)
{
  // fill out what is necessary
}

PIAna::PIMichelEnergyAnalyzer::~PIMichelEnergyAnalyzer()
{
  // fill out what is necessary
}

void PIAna::PIMichelEnergyAnalyzer::Begin()
{
  // always call Begin() of base class first to ensure things are initialized.
  PIEventAnalyzer::Begin();
  // always check if manager is available.
  if (!PIEventAction::mgr_) {
    Fatal("PIAna::PIMichelEnergyAnalyzer", "PIJobManager is not set.");
  }
  if (PIEventAction::mgr_->out_file()) {
    PIEventAnalyzer::outputfile_ = PIEventAction::mgr_->out_file();
  } else {
    Fatal("PIAna::PIMichelEnergyAnalyzer::Begin()", "Output file is not properly initialized.");
  }
  // fill out what is necessary from here.
  h_e_michel_ = new TH1D(
      "h_e_michel", "e^{+} energy from muon DAR;energy (MeV);events / 0.5 MeV",
      150, 0, 75);
  h_e_michel_->SetDirectory(PIEventAnalyzer::outputfile_);

  // give a summary of parameters at the end.
  if (PIEventAction::verbose_) {
    report();
  }
}

void PIAna::PIMichelEnergyAnalyzer::DoAction(PIEventData& event)
{
  // Do not modify this function.
  PIEventAnalyzer::DoAction(event);
}

void PIAna::PIMichelEnergyAnalyzer::End()
{
  // fill out what is necessary from here.

  // always call End() of base class at the end.
  PIEventAnalyzer::End();
}

void PIAna::PIMichelEnergyAnalyzer::analyze(const PIEventData& event)
{
  // this method is called in PIEventAnalyzer::DoAction().
  // fill out what is necessary here.
  const auto &eventinfo = event.Get<const PIMCInfo*>("info");
  const auto &decays = event.Get<const TClonesArray*>("decay");
  for (Long64_t idecay = 0; idecay != decays->GetEntries(); ++idecay) {
      auto decay = (PIMCDecay *)decays->At(idecay);
      if (decay->GetMotherPDGID() != PIAna::mup_pdgid) {
          continue;
      }
      if (decay->GetMotherEnergy() > ke_threshold_) {
          // Warning("PIAna::PIMichelEnergyAnalyzer",
          //        ::Form("muon decay energy > %f MeV", ke_threshold_));
          continue;
      }
      const auto& dauids = decay->GetDaughterPDGIDs();

      const auto ndaus = decay->GetNDaughters();
      // mu->e+mu+mu
      // not including gammas
      if (ndaus != 3) {
          continue;
      }
      for (unsigned int i=0; i !=ndaus; ++i) {
          const auto id = decay->GetDaughterPDGIDAt(i);
          if (id == PIAna::ep_pdgid) {
              const auto p3 = decay->GetDaughterMomAt(i);
              const auto ke = std::sqrt(p3.Mag2() + PIAna::m_e*PIAna::m_e) - PIAna::m_e;
              h_e_michel_->Fill(ke);
          }
      }
  }
}


void PIAna::PIMichelEnergyAnalyzer::report()
{
  // fill out what is necessary from here.
}
