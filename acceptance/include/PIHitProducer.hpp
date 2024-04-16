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

    /**
     * Set the value of merge_hit_dtmin in ns.
     * Merge two hits if dt < merge_hit_dtmin
     */
    void merge_hit_dtmin(const double dt) { merge_hit_dtmin_ = dt; }
    double merge_hit_dtmin() const { return merge_hit_dtmin_; }
    /**
     * Set the value of step_limit in mm.
     * The maximum step size when dividing G4Hit to smaller steps.
     */
    void step_limit(const double sl) { step_limit_ = sl; }
    double step_limit() { return step_limit_; }
    /**
     * Set the value of g4_step_limit in mm.
     * The maximum size of each G4Hit.
     * A warning will be issued when dealing with too large step size.
     */
    void g4_step_limit(const double sl) { g4_step_limit_ = sl; }
    double g4_step_limit() const { return g4_step_limit_; }

    void de_thres(const double thres) { de_thres_ = thres; }
    double de_thres() const { return de_thres_; }

  protected:
    void produce(PIEventData &) override;
    void fill_dummy(PIEventData &) override;
    void report() override;

  private:
    std::unique_ptr<PIAnaG4StepDivider> divider_;
    std::unique_ptr<PIAnaHitMerger> merger_;

    double merge_hit_dtmin_; // ns
    double step_limit_; // mm
    double g4_step_limit_; // mm

    double de_thres_;
  };
};
#endif
