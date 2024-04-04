#include "PIEventAnalyzer.hpp"
#include "PIEventData.hpp"

#include "TError.h"

PIAna::PIEventAnalyzer::PIEventAnalyzer(const std::string &name)
                                    : name_(name) {}

PIAna::PIEventAnalyzer::~PIEventAnalyzer() {}

void PIAna::PIEventAnalyzer::Begin()
{
  if (!PIEventAction::initialized_) {
    PIEventAction::initialize();
  }
}

void PIAna::PIEventAnalyzer::DoAction(PIEventData &evt)
{
  if (!PIEventAction::initialized_) {
    std::string msg = "Event analyzer " + name_ + " is not initialized.\n";
    Error("PIAna::PIEventAnalyzer::DoAction", msg.c_str());
  }
  analyze(evt);
}

void PIAna::PIEventAnalyzer::End()
{
  if (!PIEventAction::initialized_) {
    std::string msg = "Event analyzer " + name_ + " is not initialized.\n";
    Error("PIAna::PIEventFilter::End", msg.c_str());
  }
}
