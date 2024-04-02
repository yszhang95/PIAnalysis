#ifndef __PI_PiDAR_Filter__
#define __PI_PiDAR_Filter__
#include "PIEventFilter.hpp"
#include "PIEventData.hpp"
#include <Rtypes.h>

namespace PIAna
{
  class  PIPiDARFilter: public PIEventFilter
  {
  public:
    PIPiDARFilter(const std::string& n, const int code);
    ~PIPiDARFilter() override;
    void Begin() override;
    void DoAction(PIEventData&) override;
    void End() override;
    double ke_threshold() { return ke_threshold_; }
    void ke_threshold(const double thres) { ke_threshold_ = thres; }

  protected:
    bool get_bit(const PIEventData &evt) override;

  private:
    double ke_threshold_;
};
};

#endif
