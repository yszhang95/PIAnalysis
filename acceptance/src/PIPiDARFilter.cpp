#include "PIPiDARFilter.hpp"

#include "PIMCTrack.hh"

#include "TError.h"
#include "TClonesArray.h"

#include <vector>

PIPiDARFilter::PIPiDARFilter()
{}

PIPiDARFilter::~PIPiDARFilter()
{
}

bool PIPiDARFilter::get_bit()
{
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
        const auto m = 0.13957E3;
        const auto e2 = px * px + py * py + pz * pz + m * m;
        const auto KE = std::sqrt(e2) - m;
        if (KE < 0.001 && !stopped) {
          stopped = true;
          return stopped;
        }
      }
    }
  }

  return false;
}
