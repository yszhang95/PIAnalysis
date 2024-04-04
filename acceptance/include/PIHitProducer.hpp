#ifndef __PI_HitProducer__
#define __PI_HitProducer__
#include <memory>

#include "PIEventProducer.hpp"

class PIAnaG4StepDivider;
class PIAnaHitMerger;

namespace PIAna
{
  class PIHitProducer : public PIEventProducer
  {
  public:
    PIHitProducer(const std::string &);
    ~PIHitProducer();

    void Begin() override;
    void DoAction(PIEventData &evt) override;
    void End() override;


    void merge_hit_dtmin(const double dt) { merge_hit_dtmin_ = dt; }
    double merge_hit_dtmin() const { return merge_hit_dtmin_; }
  protected:
    void produce(PIEventData &) override;
    void fill_dummy(PIEventData &) override;

  private:
    std::unique_ptr<PIAnaG4StepDivider> divider_;
    std::unique_ptr<PIAnaHitMerger> merger_;

    double merge_hit_dtmin_;
  };
};
#endif
