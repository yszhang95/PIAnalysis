#include "PIFilterBase.hpp"
#include "PIMCAtar.hh"
#include "PIMCDecay.hh"
#include "PIMCInfo.hh"

#include "PIMCTrack.hh"
#include "TClonesArray.h"
#include <stdexcept>

PIFilterBase::PIFilterBase()
    : info_(nullptr), track_(nullptr), atar_(nullptr),
      decay_(nullptr), loaded_(false)
{}

PIFilterBase::~PIFilterBase()
{
  info_ = nullptr;
  track_ = nullptr;
  atar_ = nullptr;
  decay_ = nullptr;
}

void PIFilterBase::load_event(PIMCInfo *info, TClonesArray *track,
                             TClonesArray *atar, TClonesArray* decay)
{
  if (loaded_) {
    throw std::logic_error("Event content has already been retrieved.\n");
    return;
  }
  info_ = info;
  track_ = track;
  atar_ = atar;
  decay_ = decay;
  loaded_ = true;
  return;
}

bool PIFilterBase::eval_bit()
{
  if (!loaded_) {
    throw std::logic_error("Event content has not been retrieved.\n");
  }
  const bool result = get_bit();
  loaded_ = false;
  return result;
}
