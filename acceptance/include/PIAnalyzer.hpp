#ifndef __PI_Analyzer__
#define __PI_Analyzer__

#include "PIAnaEvtBase.hpp"
#include <Rtypes.h>
#include <memory>
#include <string>
#include <vector>

// #include "TTree.h"
class TChain;
class TH1D;
class TTree;
class TFile;
class TClonesArray;

class PIAnaHit;
class PIAnaHitMerger;
class PIAnaG4StepDivider;

class PIMCInfo;

class PIAnalyzer : public PIAnaEvtBase
{
public:
  PIAnalyzer(const std::string&);
  ~PIAnalyzer();
  void begin() override;
  void run() override;
  void end() override;

  ClassDefOverride(PIAnalyzer, 1)

private:
  void clear();
  int analyze(long long);

  int analyze_atar_hits();

  TTree *t_;

  TH1D *h_pi_fake_;
  TH1D *h_pi_true_;
  TH1D *h_pi_all_;

  TH1D *h_e_fake_;
  TH1D *h_e_true_;
  TH1D *h_e_all_;

  TH1D *h_prompt_;
  TH1D *h_nonprompt_;

  std::unique_ptr<PIAnaG4StepDivider> divider_;
  std::unique_ptr<PIAnaHitMerger> merger_;

  // double pi_stop_x_;
  // double pi_stop_y_;
  // double pi_stop_z_;
  static constexpr ushort NHITS_MAX_ = 100;
  Short_t hi_xstrip_[NHITS_MAX_];
  Short_t hi_ystrip_[NHITS_MAX_];
  Short_t hi_zlayer_[NHITS_MAX_];
  Float_t hi_edep_[NHITS_MAX_];
  Float_t hi_t_[NHITS_MAX_];
  UShort_t hi_nhits_;

  Short_t pi_stop_xstrip_;
  Short_t pi_stop_ystrip_;
  Short_t pi_stop_zlayer_;
  Bool_t pi_stop_found_;

};

#endif
