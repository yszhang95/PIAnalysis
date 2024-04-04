#include "TClonesArray.h"

#include "PIHitProducer.hpp"
#include "PIAnaEvtBase.hpp"
#include "PIAnaHit.hpp"
#include "PIEventData.hpp"

PIAna::PIHitProducer::PIHitProducer(const std::string &name)
  : PIEventProducer(name), merge_hit_dtmin_(1), step_limit_(0.02), g4_step_limit_(60)
{
  divider_ = std::make_unique<PIAnaG4StepDivider>();
  merger_ = std::make_unique<PIAnaHitMerger>();
}

PIAna::PIHitProducer::~PIHitProducer() {}

void PIAna::PIHitProducer::Begin()
{
  PIEventProducer::Begin();
  divider_->step_limit(step_limit_);
  divider_->g4_step_limit(g4_step_limit_);
  merger_->dt_min(merge_hit_dtmin_);

  if (verbose_) {
    report();
  }
}

void PIAna::PIHitProducer::DoAction(PIEventData &event)
{
  PIEventProducer::DoAction(event);
}

void PIAna::PIHitProducer::End() { PIEventProducer::End(); }

void PIAna::PIHitProducer::produce(PIEventData &event)
{
  std::vector<PIAnaHit> rec_hits;
  const TClonesArray* atar = event.Get<const TClonesArray*>("atar");
  for (int j=0; j<atar->GetEntries(); ++j) {
    auto atar_hit = dynamic_cast<PIMCAtar*>(atar->At(j));
    auto hits = divider_->process_atar_hit(*atar_hit);
    auto merged_hits = merger_->merge(hits);

    std::move(merged_hits.begin(), merged_hits.end(),
              std::back_inserter(rec_hits));
  }

  rec_hits = merger_->merge(rec_hits);

  std::sort(rec_hits.begin(), rec_hits.end(),
            [](const PIAnaHit &hit1, const PIAnaHit &hit2) {
              return hit1.t() < hit2.t();
            });
  event.Put<std::vector<PIAnaHit > >(PIEventProducer::GetName(), rec_hits);
}

void PIAna::PIHitProducer::fill_dummy(PIEventData& event) {}

void PIAna::PIHitProducer::report()
{
  Info("PIAna::PIHitProducer",
       ::Form("PIHitProducer: %s", PIEventProducer::GetName().c_str()));
  Info("PIAna::PIHitProducer",
       ::Form("Merge two hits if dt < %f [ns]", merge_hit_dtmin_));
  Info("PIAna::PIHitProducer",
       ::Form("Maximum step size for each divided step: %f [mm]", step_limit_));
  Info("PIAna::PIHitProducer",
       ::Form("Maximum step size of each G4 step for warning: %f [mm]",
              g4_step_limit_));
}
