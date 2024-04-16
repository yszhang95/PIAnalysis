#include "PIAnaConst.hpp"
#include "PIEventData.hpp"
#include "PIEventFilter.hpp"
#include "PIFilterPiDAR.hpp"

#include "PIMCTrack.hh"

#include "TError.h"
#include "TClonesArray.h"

#include <vector>

PIAna::PIPiDARFilter::PIPiDARFilter(const std::string& name, const int code)
  : PIEventFilter(name, code), ke_threshold_(1E-3)
{}

PIAna::PIPiDARFilter::~PIPiDARFilter()
{
}

void PIAna::PIPiDARFilter::Begin()
{
  PIEventFilter::Begin();
}
void PIAna::PIPiDARFilter::DoAction(PIEventData& event)
{
  PIEventFilter::DoAction(event);
}
void PIAna::PIPiDARFilter::End()
{
  PIEventFilter::End();
}

bool PIAna::PIPiDARFilter::filter(const PIEventData& event)
{
  const TClonesArray* tracks = event.Get<const TClonesArray*>("track");
  for (size_t j=0; j<tracks->GetEntries(); ++j) {
    auto g4track = dynamic_cast<PIMCTrack*>(tracks->At(j));
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
          Error("PIAna::PIPiDARFilter::filter",
                "The arrays of pion momentum are not sorted by time.");
        }
        const auto px = pxs.at(i);
        const auto py = pys.at(i);
        const auto pz = pzs.at(i);
        const auto m = PIAna::m_pi;
        const auto e2 = px * px + py * py + pz * pz + m * m;
        const auto KE = std::sqrt(e2) - m;
        if (KE < ke_threshold_ && !stopped) {
          stopped = true;
          return stopped;
        }
      }
    }
  }

  return false;
}
