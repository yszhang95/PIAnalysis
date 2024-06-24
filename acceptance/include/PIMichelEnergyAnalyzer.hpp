/*********************************************************************
*                                                                    *
* 2024-06-24 08:18:02                                                *
* acceptance/include/PIMichelEnergyAnalyzer.hpp                      *
*                                                                    *
*********************************************************************/

#ifndef __PI_MichelEnergyAnalyzer__
#define __PI_MichelEnergyAnalyzer__

#include "PIEventAnalyzer.hpp"

class TH1D;

namespace PIAna
{
  class PIMichelEnergyAnalyzer : public PIEventAnalyzer
  {
  public:
    PIMichelEnergyAnalyzer(const std::string&);
    ~PIMichelEnergyAnalyzer();

    void Begin() override;
    void DoAction(PIEventData&) override;
    void End() override;

    double ke_threshold() { return ke_threshold_; }
    void ke_threshold(const double thres) { ke_threshold_ = thres; }

  protected:
    void analyze(const PIEventData&) override;
    void report() override;

  private:
    TH1D* h_e_michel_;
    double ke_threshold_;
  };
};
#endif
