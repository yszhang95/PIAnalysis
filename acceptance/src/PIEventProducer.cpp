#include "TError.h"

#include "PIEventProducer.hpp"
#include "PIEventData.hpp"

PIAna::PIEventProducer::PIEventProducer(const std::string &name)
                                    : name_(name) {}

PIAna::PIEventProducer::~PIEventProducer() {}

void PIAna::PIEventProducer::Begin()
{
  if (!PIEventAction::initialized_) {
    PIEventAction::initialize();
  }
}

void PIAna::PIEventProducer::DoAction(PIEventData &evt)
{
  if (!PIEventAction::initialized_) {
    std::string msg = "Event producer " + name_ + " is not initialized.\n";
    Error("PIAna::PIEventFilter::DoAction", msg.c_str());
  }

  produce(evt);
}

void PIAna::PIEventProducer::End()
{
 if (!PIEventAction::initialized_) {
    std::string msg = "Event producer " + name_ + " is not initialized.\n";
    Error("PIAna::PIEventFilter::DoAction", msg.c_str());
  }
}
