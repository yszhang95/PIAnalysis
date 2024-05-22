/*********************************************************************
*                                                                    *
* 2024-04-30 15:08:28                                                *
* acceptance/src/PITrueMomProducer.cpp                               *
*                                                                    *
*********************************************************************/

#include "TError.h"
#include "TClonesArray.h"
#include "Math/Vector3D.h"

#include "PIMCDecay.hh"

#include "PIJobManager.hpp"
#include "PIEventData.hpp"
#include "PITrueMomProducer.hpp"
#include <sstream>

PIAna::PITrueMomProducer::PITrueMomProducer(const std::string &name)
    : PIEventProducer(name), pip_pdgid_(211), pim_pdgid_(-211), mum_pdgid_(13),
      mup_pdgid_(-13), em_pdgid_(11), ep_pdgid_(-11), gamma_pdgid_(22),
      pi_mode_(PiDecayMode::undefined)
{
  // fill out what is necessary
}

PIAna::PITrueMomProducer::~PITrueMomProducer()
{
  // fill out what is necessary
}

void PIAna::PITrueMomProducer::Begin()
{
  // always call Begin() of base class first to ensure things are initialized.
  PIEventProducer::Begin();
  // always check if manager is available.
  if (!PIEventAction::mgr_) {
    Fatal("PIAna::PITrueMomProducer", "PIJobManager is not set.");
  }

  // fill out what is necessary from here.
  if (pi_mode_ == PiDecayMode::undefined) {
    Fatal("PIAna::PITrueMomProducer", "decay channel of pions is undefined.");
  }
  // give a summary of parameters at the end.
  if (PIEventAction::verbose_) {
    report();
  }
}

void PIAna::PITrueMomProducer::DoAction(PIEventData& event)
{
  // Do not modify this function.
  PIEventProducer::DoAction(event);
}

void PIAna::PITrueMomProducer::End()
{
  // fill out what is necessary from here.

  // always call End() of base class at the end.
  PIEventProducer::End();
}

void PIAna::PITrueMomProducer::produce(PIEventData& event)
{
  // this method is called in PIEventProducer::DoAction().
  // When no product can be made, call customized fill_dummy().
  // fill out what is necessary.

  ROOT::Math::XYZVector emom;

  const TClonesArray *decays = event.Get<const TClonesArray *>("decay");

  for (Long64_t ipar = 0; ipar != decays->GetEntries(); ++ipar) {
    auto particle = (PIMCDecay *)decays->At(ipar);

    const bool ispiplus = particle->GetMotherPDGID() == pip_pdgid_
      && pi_mode_ == PiDecayMode::pienu;
    const bool ismuplus = particle->GetMotherPDGID() == mup_pdgid_
      && pi_mode_ == PiDecayMode::pimue;
    if (ispiplus || ismuplus) {
      for (int idau = 0; idau < particle->GetNDaughters(); ++idau) {
        if (particle->GetDaughterPDGIDAt(idau) == ep_pdgid_) {
          emom = particle->GetDaughterMomAt(idau);
        }
      }
    }
  }

  if (emom.Mag2() < 1E-9) {
    Warning("PIAna::PITrueMomProducer", "|P|^2 of positron < 1E-9");
  } else {
    if (PIEventAction::verbose_) {
      Info("PIAna::PITrueMomProducer",
           ::Form("Positron momentum (%f, %f, %f)", emom.X(), emom.Y(), emom.Z()));
    }
  }

  event.Put<ROOT::Math::XYZVector>(
      ::Form("%s_emom", PIEventProducer::GetName().c_str()), emom);
}

void PIAna::PITrueMomProducer::fill_dummy(PIEventData& event)
{
  // fill dummy values here.
}

void PIAna::PITrueMomProducer::report()
{
  // fill out what is necessary from here.
}
