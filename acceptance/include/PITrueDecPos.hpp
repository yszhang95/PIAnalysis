/*********************************************************************
*                                                                    *
* 2024-04-05 13:36:35                                                *
* acceptance/include/PITrueDecPos.hpp                                *
*                                                                    *
*********************************************************************/

#ifndef __PI_TrueDecPos__
#define __PI_TrueDecPos__

#include "PIEventProducer.hpp"

namespace PIAna
{
  class PITrueDecPos : public PIEventProducer
  {
  public:
    PITrueDecPos(const std::string&);
    ~PITrueDecPos();

    void Begin() override;
    void DoAction(PIEventData&) override;
    void End() override;

  protected:
    void produce(PIEventData&) override;
    void fill_dummy(PIEventData&) override;
    void report() override;

  private:
    const int pip_pdgid_; // pi+
    const int pim_pdgid_; // pi-
    const int mup_pdgid_; // mu+
    const int mum_pdgid_; // mu-
    const int ep_pdgid_;  // e+
    const int em_pdgid_;  // e-
    const int gamma_pdgid_; // gamma
  };
};
#endif
