#ifndef __PI_PiDAR_Filter__
#define __PI_PiDAR_Filter__
#include "PIFilterBase.hpp"

class PIPiDARFilter : public PIFilterBase
{
public:
  PIPiDARFilter();
  ~PIPiDARFilter() override;
protected:
  bool get_bit() override;
};
#endif
