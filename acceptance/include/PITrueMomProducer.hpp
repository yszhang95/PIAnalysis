/*********************************************************************
*                                                                    *
* 2024-04-30 15:08:28                                                *
* acceptance/include/PITrueMomProducer.hpp                           *
*                                                                    *
*********************************************************************/

#ifndef __PI_TrueMomProducer__
#define __PI_TrueMomProducer__

#include "PIEventProducer.hpp"

namespace PIAna
{
  class PITrueMomProducer : public PIEventProducer
  {
  public:
    enum class PiDecayMode { undefined, pienu, pimue };
    PITrueMomProducer(const std::string&);
    ~PITrueMomProducer();

    void Begin() override;
    void DoAction(PIEventData&) override;
    void End() override;

    void decay_mode(PiDecayMode mode) { pi_mode_ = mode; }

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
    const int gamma_pdgid_; // gamm
    PiDecayMode pi_mode_;
  };
};
#endif
