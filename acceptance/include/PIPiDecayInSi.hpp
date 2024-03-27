#ifndef __PI_PiDecayInSi__
#define __PI_PiDecayInSi__
#include "PIFilterBase.hpp"
#include <Rtypes.h>

class PIPiDecayInSi : public PIFilterBase
{
public:
  PIPiDecayInSi(const int evtcode, const int lower, const int upper);
  ~PIPiDecayInSi() override;

protected:
  bool get_bit() override;
private:
  int lower_;
  int upper_;
};
#endif
