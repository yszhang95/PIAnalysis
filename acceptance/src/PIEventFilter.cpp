#include "PIEventFilter.hpp"
#include "PIEventData.hpp"

#include <Rtypes.h>
#include <stdexcept>
#include "TError.h"

PIAna::PIEventFilter::PIEventFilter(const std::string name, const int code)
  : name_(name), code_(code), use_(true)
{}

PIAna::PIEventFilter::~PIEventFilter() {}

void PIAna::PIEventFilter::Begin()
{
  if (!PIEventAction::initialized_) {
    PIEventAction::initialize();
  }
}

void PIAna::PIEventFilter::DoAction(PIEventData& event)
{
  if (!PIEventAction::initialized_) {
    std::string msg = "Event filter " + name_ + " is not initialized.\n";
    Error("PIAna::PIEventFilter::DoAction", msg.c_str());
  }

  const bool result = filter(event);
  event.Put<bool>(name_, result);
  std::string codename = name_ + "_code";
  event.Put<int>(codename, code_);
  return;
}

void PIAna::PIEventFilter::End()
{
  if (!PIEventAction::initialized_) {
    std::string msg = "Event filter " + name_ + " is not initialized.\n";
    Error("PIAna::PIEventFilter::End", msg.c_str());
  }
}
