#include "PIEventAnalyzer.hpp"
#include "PIEventData.hpp"

PIAna::PIEventAnalyzer::PIEventAnalyzer(const std::string &name)
                                    : name_(name) {}

PIAna::PIEventAnalyzer::~PIEventAnalyzer() {}

void PIAna::PIEventAnalyzer::DoAction(PIEventData &evt)
{
  analyze(evt);
}
