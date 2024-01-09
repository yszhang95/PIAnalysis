#ifndef __PI_Analyzer__
#define __PI_Analyzer__

#include <RtypesCore.h>
#include <TClonesArray.h>
#include <memory>
#include <string>
#include <sys/types.h>
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

class PIAnalyzer
{
public:
  PIAnalyzer(const std::string&);
  ~PIAnalyzer();
  void begin();
  void run();
  void end();

  void treename(std::string const& n);
  template<typename InputIter>
void filenames(InputIter first, InputIter last);
  void add_file(const std::string&);
  void add_friend(const std::string&);

  ClassDef(PIAnalyzer, 1)

private:
  void clear();
  int analyze(long long);

  int analyze_atar_hits();

  std::unique_ptr<TChain> chain_;
  std::unique_ptr<TFile> fout_;
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

  PIMCInfo* info_;
  TClonesArray* track_;
  TClonesArray* atar_;

  std::string treename_;
  std::vector<std::string> filenames_;
  std::vector<std::string> ftreenames_;

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

  bool initialized_;
};

#endif
