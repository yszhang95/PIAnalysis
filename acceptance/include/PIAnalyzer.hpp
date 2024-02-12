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
class PITCluster;

class PIMCInfo;

class PIAnalyzer : public PIAnaEvtBase
{
public:
  PIAnalyzer(const std::string&);
  ~PIAnalyzer();
  void begin() override;
  void run() override;
  void end() override;

  void min_tcluster(const unsigned int n) { min_tcluster_ = n; }
  const unsigned int min_tcluster() const { return min_tcluster_; }

  ClassDefOverride(PIAnalyzer, 1)

private:
  void clear();

  int analyze_atar_hits();

  enum EvtCode {
    PiDAR = 0,
    PiDIF,
    PiDARNoFirstLayerHit,
    PiDAROffCent,
    PiDARMergedT,
    PiWrongT
  };

  TTree *t_;
  TH1D* hcategories_;
  TH1D *htcluster_;
  TH1D *hdelayed_;

  TH1D *hdelayed_true_;
  TH1D *hdecay_true_;
  TH1D *hdecay_volume_true_;

  std::unique_ptr<PIAnaG4StepDivider> divider_;
  std::unique_ptr<PIAnaHitMerger> merger_;
  std::unique_ptr<PITCluster> tcluster_;

  unsigned int min_tcluster_;
  // double pi_stop_x_;
  // double pi_stop_y_;
  // double pi_stop_z_;
  // static constexpr ushort NHITS_MAX_ = 100;
  // Short_t hi_xstrip_[NHITS_MAX_];
  // Short_t hi_ystrip_[NHITS_MAX_];
  // Short_t hi_zlayer_[NHITS_MAX_];
  // Float_t hi_edep_[NHITS_MAX_];
  // Float_t hi_t_[NHITS_MAX_];
  // UShort_t hi_nhits_;

  // Short_t pi_stop_xstrip_;
  // Short_t pi_stop_ystrip_;
  // Short_t pi_stop_zlayer_;
  // Bool_t pi_stop_found_;

};

#endif
