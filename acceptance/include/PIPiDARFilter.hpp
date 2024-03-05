#ifndef __PI_PiDAR_Filter__
#define __PI_PiDAR_Filter__
#include "PIFilterBase.hpp"
#include <Rtypes.h>

class PIPiDARFilter : public PIFilterBase
{
public:
  PIPiDARFilter(const int);
  ~PIPiDARFilter() override;
    double ke_threshold() { return ke_threshold_; }
  void ke_threshold(const double thres) { ke_threshold_ = thres; }

protected:
  bool get_bit() override;
private:
  double ke_threshold_;
};
#endif
