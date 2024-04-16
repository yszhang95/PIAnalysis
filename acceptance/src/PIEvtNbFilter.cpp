#include "PIAnaConst.hpp"
#include "PIEventData.hpp"
#include "PIEventFilter.hpp"
#include "PIEvtNbFilter.hpp"

#include "PIMCInfo.hh"

#include "TError.h"

#include <string>
#include <vector>
#include <sstream>
#include <iostream>

PIAna::PIEvtNbFilter::PIEvtNbFilter(const std::string& name, const int code)
  : PIEventFilter(name, code)
{}

PIAna::PIEvtNbFilter::~PIEvtNbFilter()
{
}

void PIAna::PIEvtNbFilter::Begin()
{
  PIEventFilter::Begin();
}
void PIAna::PIEvtNbFilter::DoAction(PIEventData& event)
{
  PIEventFilter::DoAction(event);
}
void PIAna::PIEvtNbFilter::End()
{
  PIEventFilter::End();
}

bool PIAna::PIEvtNbFilter::filter(const PIEventData& event)
{
  const PIMCInfo* info = event.Get<const PIMCInfo*>("info");
  return selector_(info->GetRun(), info->GetEvent(), info->GetEventID());
}

void PIAna::PIEvtNbFilter::load_filter(const std::string & ids) {
  if (PIEventAction::initialized_) {
    std::string msg = "Event Filter " + PIEventFilter::GetName() + " has been initialized.\n";
    Error("PIAna::PIEvtNbFilter::load_filter", msg.c_str());
  }
  selector_.load_event_id(ids);
}

void PIAna::PIEvtNbFilter::select_event_id::load_event_id(
    const std::string &ids) {
  std::istringstream idstreams(ids);
  std::string id;
  while (std::getline(idstreams, id, ',')) {
    // std::cout << id << "\n";
    std::istringstream idstream(id);
    std::string nb;
    std::vector<int> nbs;
    while (std::getline(idstream, nb, ':')) {
      nbs.push_back(std::stoi(nb));
      // std::cout << nbs.back() << "\n";
    }
    if (nbs.size() != 3) {
      std::string msg = idstream.str() + " is in wrong format.";
      Warning("PIAna::PIEvtNbFilter::select_event_id::load_event_id",
              msg.c_str());
      continue;
    }
    ids_.emplace_back(nbs.at(0), nbs.at(1), nbs.at(2));
  }
  std::string msg = "Will select";
  for (const auto id : ids_) {
    msg += ", " + std::to_string(id.run) + ":" + std::to_string(id.event) + ":" + std::to_string(id.eventid);
  }
  Info("PIAna::PIEvtNbFilter::select_event_id::load_event_id", msg.c_str());
}

bool PIAna::PIEvtNbFilter::select_event_id::operator()(const int run,
                                                       const int event,
                                                       const int eventid)
{
  for (const auto &id : ids_) {
    bool passed = id.run == run && id.event == event && id.eventid == eventid;
    if (passed)
      return true;
  }
  return false;
}
