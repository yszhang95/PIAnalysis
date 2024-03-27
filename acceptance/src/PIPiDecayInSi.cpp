#include "PIAnaConst.hpp"
#include "PIPiDecayInSi.hpp"

#include "PIMCDecay.hh"

#include "TError.h"
#include "TClonesArray.h"

#include <vector>

PIPiDecayInSi::PIPiDecayInSi(const int evtcode,
                             const int lower, const int upper)
  : PIFilterBase(evtcode), lower_(lower), upper_(upper)
{}

PIPiDecayInSi::~PIPiDecayInSi()
{
}

bool PIPiDecayInSi::get_bit()
{
  for (size_t j = 0; j < decay_->GetEntries(); ++j) {
    // decay
    auto decay_info_ = dynamic_cast<PIMCDecay *>(decay_->At(j));
    // assume there is only one pion in each event
    if (decay_info_->GetMotherPDGID() == 211) {
      const int vol = decay_info_->GetVolume();
      if (vol >= lower_ && vol < upper_) {
        return true;
      }
    }
  }

  return false;
}
