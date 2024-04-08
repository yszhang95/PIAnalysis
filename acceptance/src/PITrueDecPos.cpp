/*********************************************************************
*                                                                    *
* 2024-04-05 13:36:35                                                *
* acceptance/src/PITrueDecPos.cpp                                    *
*                                                                    *
*********************************************************************/

#include "TError.h"
#include "TClonesArray.h"
#include <Math/Point3Dfwd.h>
#include "Math/Point3D.h"
#include "Math/Vector3D.h"

#include "PIMCTrack.hh"

#include "PIJobManager.hpp"
#include "PIEventData.hpp"
#include "PITrueDecPos.hpp"
#include "PIPointWrapper.hpp"

PIAna::PITrueDecPos::PITrueDecPos(const std::string &name)
    : PIEventProducer(name), pip_pdgid_(211), pim_pdgid_(-211),
      mum_pdgid_(13), mup_pdgid_(-13),
      em_pdgid_(11), ep_pdgid_(-11), gamma_pdgid_(22)
{
  // fill out what is necessary
}

PIAna::PITrueDecPos::~PITrueDecPos()
{
  // fill out what is necessary
}

void PIAna::PITrueDecPos::Begin()
{
  // always call Begin() of base class first to ensure things are initialized.
  PIEventProducer::Begin();
  // always check if manager is available.
  if (!PIEventAction::mgr_) {
    Fatal("PIAna::PITrueDecPos", "PIJobManager is not set.");
  }

  // fill out what is necessary from here.

  // give a summary of parameters at the end.
  if (PIEventAction::verbose_) {
    report();
  }
}

void PIAna::PITrueDecPos::DoAction(PIEventData& event)
{
  // Do not modify this function.
  PIEventProducer::DoAction(event);
}

void PIAna::PITrueDecPos::End()
{
  // fill out what is necessary from here.

  // always call End() of base class at the end.
  PIEventProducer::End();
}

void PIAna::PITrueDecPos::produce(PIEventData& event)
{
  // this method is called in PIEventProducer::DoAction().
  // When no product can be made, call customized fill_dummy().
  // fill out what is necessary.
  XYZPointWrapper pivertex;
  XYZPointWrapper estart;
  const TClonesArray *tracks = event.Get<const TClonesArray *>("track");
  for (Long64_t itrack = 0; itrack != tracks->GetEntries(); ++itrack) {
    auto track = (PIMCTrack *)tracks->At(itrack);
    const bool ispiplus = track->GetPDGID() == pip_pdgid_;
    const bool ispositron = track->GetPDGID() == ep_pdgid_;
    if (ispiplus) {
      pivertex = ROOT::Math::XYZPoint(track->GetPostX().back(),
                                      track->GetPostY().back(),
                                      track->GetPostZ().back());
      if (PIEventAction::verbose_) {
        Info("PIAna::PITrueDecPos",
             ::Form("Process in the last step of pion is %d."
                    " Its momentum is (%f, %f, %f) MeV.",
                    track->GetProcessID().back(), track->GetPostMomX().back(),
                    track->GetPostMomY().back(), track->GetPostMomZ().back()));
      }
    }
    if (ispositron) {
      const bool isfrompi = track->GetParentID();
      estart = ROOT::Math::XYZPoint(track->GetPreX(), track->GetPreY(),
                                    track->GetPreZ());
      if (PIEventAction::verbose_) {
        Info("PIAna::PITrueDecPos", ::Form("Parent process of positron is %d",
                                           track->GetParentProcess()));
      }
    }

    if (!pivertex.null() && !estart.null()) {
      break;
    }
  }
  if (!pivertex.null() && !estart.null()) {
    if (PIEventAction::verbose_) {
      auto dist = (pivertex.point() - estart.point()).Mag2();
      Info("PIAna::PITrueDecPos",
           ::Form("Distance^2 from pi+ vertex to e+ production is %f [mm^2]",
                  dist));
    }
  } else {
    Error("PIAna::PITrueDecPos", "Neither found pi+ nor e+.");
  }

  event.Put<XYZPointWrapper>(
      ::Form("%s_pivertex", PIEventProducer::GetName().c_str()), pivertex);
  event.Put<XYZPointWrapper>(
      ::Form("%s_estart", PIEventProducer::GetName().c_str()), estart);
}

void PIAna::PITrueDecPos::fill_dummy(PIEventData& event)
{
  // fill dummy values here.
  // XYZPointWrapper has a flag of null;
}

void PIAna::PITrueDecPos::report()
{
  // fill out what is necessary from here.
}
