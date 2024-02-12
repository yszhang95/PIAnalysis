#ifndef __PI_ANA_Atar_Pho__
#define __PI_ANA_Atar_Pho__
#include "PIAnaEvtBase.hpp"
#include <Rtypes.h>

#include <map>

class PIAnaAtarPho : public PIAnaEvtBase
{
public:
  PIAnaAtarPho(const std::string &treename);
  ~PIAnaAtarPho();

  void begin() override;
  void run() override;
  void end() override;

  /**
     Filter run number, event number, event id.
     The string is in format of "run:event:id", separated by ":".
   */

  ClassDefOverride(PIAnaAtarPho, 1)

private:
  int analyze();
  int analyze_atarpho();

  std::map<int, unsigned long long> procs_;
  std::map<int, unsigned long long> edep_procs_;

};
#endif
