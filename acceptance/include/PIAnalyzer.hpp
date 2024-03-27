#ifndef __PI_Analyzer__
#define __PI_Analyzer__

#include "Math/Vector3Dfwd.h"

#include "PIAnaCluster.hpp"
#include "PIAnaEvtBase.hpp"
#include <Rtypes.h>
#include <memory>
#include <string>
#include <vector>

// #include "TTree.h"
class TChain;
class TH1D;
class TH2D;
class TTree;
class TFile;
class TClonesArray;

class PIAnaHit;
class PIAnaHitMerger;
class PIAnaG4StepDivider;
class PITCluster;
class PIXYZCluster;

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

  void merge_hit_dtmin(const double dt) { merge_hit_dtmin_ = dt; }
  double merge_hit_dtmin() const { return merge_hit_dtmin_; }

  void tcluster_dt(const double dt) { tcluster_dt_ = dt; }
  double tcluster_dt() const { return tcluster_dt_; }

  void pi_dt1(const double dt) { pi_dt1_ = dt; }
  void pi_dt2(const double dt) { pi_dt2_ = dt; }
  void mu_dt1(const double dt) { mu_dt1_ = dt; }
  void mu_dt2(const double dt) { mu_dt2_ = dt; }
  double pi_dt1() const { return pi_dt1_; }
  double pi_dt2() const { return pi_dt2_; }
  double mu_dt1() const { return mu_dt1_; }
  double mu_dt2() const { return mu_dt2_; }

  void debug(const bool debug) { debug_ = debug; }
  bool debug() const { return debug_; };

private:
  void clear();

  int analyze_atar_hits();

  void print_hit(const PIAnaHit &);
  void print_debug_info();

  std::vector<std::pair<const PIAnaHit *, const PIAnaHit *>>
  decay_point(const std::vector<const PIAnaHit *> &,
              const std::vector<const PIAnaHit *> &);

  std::pair<bool, ROOT::Math::Polar3DVector>
  positron_direction(const std::vector<const PIAnaHit *> hits,
                     const PIAnaPointCloud::Point pivertex);

  enum EvtCode {
    PiDAR = 0,
    PiDIF,
    PiDARNoFirstLayerHit,
    PiOffATAR,
    PiDARMergedT,
    PiWrongT
  };

  TTree *rec_tree_;
  TH1D* hevtcode_;
  TH1D* hcategories_;
  TH1D *htcluster_;
  TH1D *hdelayed_;

  TH1D *hdelayed_true_;
  TH1D *hdecay_true_;
  TH1D *hdecay_volume_true_;

  TH2D *hdecay_xz_byz_;
  TH2D *hdecay_yz_byz_;
  TH2D *hdecay_xz_bye_;
  TH2D *hdecay_yz_bye_;
  TH2D *hdecay_xz_byz_2hit_;
  TH2D *hdecay_yz_byz_2hit_;
  TH2D *hdecay_xz_bye_2hit_;
  TH2D *hdecay_yz_bye_2hit_;


  TH2D *hdecay_xy_diff_;
  TH2D *hdecay_xz_diff_;
  TH2D *hdecay_yz_diff_;

  TH2D *hdecay_rec_xy_;
  TH2D *hdecay_rec_xz_;
  TH2D *hdecay_rec_yz_;

  TH2D *hdecay_rec_xy_5_ehits_;
  TH2D *hdecay_rec_xz_5_ehits_;
  TH2D *hdecay_rec_yz_5_ehits_;

  TH2D *hdecay_rec_xy_center_;
  TH2D *hdecay_rec_xz_center_;
  TH2D *hdecay_rec_yz_center_;

  TH2D *hdecay_rec_xy_5_ehits_center_;
  TH2D *hdecay_rec_xz_5_ehits_center_;
  TH2D *hdecay_rec_yz_5_ehits_center_;

  TH2D *h_e_rec_thetaphi_;

  std::unique_ptr<PIAnaG4StepDivider> divider_;
  std::unique_ptr<PIAnaHitMerger> merger_;
  std::unique_ptr<PITCluster> tcluster_;
  std::unique_ptr<PIXYZCluster> xyzcluster_;

  double pi_dt1_; // hits not merged at decay location
  double mu_dt1_; // hits not merged at decay location
  double pi_dt2_;
  double mu_dt2_;
  double merge_hit_dtmin_;
  double tcluster_dt_;

  unsigned int min_tcluster_;
  // double pi_stop_x_;
  // double pi_stop_y_;
  // double pi_stop_z_;

  double pi_decay_x_true_;
  double pi_decay_y_true_;
  double pi_decay_z_true_;

  double pi_decay_x_;
  double pi_decay_y_;
  double pi_decay_z_;
  // static constexpr ushort NHITS_MAX_ = 100;
  // Short_t hi_xstrip_[NHITS_MAX_];
  // Short_t hi_ystrip_[NHITS_MAX_];
  // Short_t hi_zlayer_[NHITS_MAX_];
  // Float_t hi_edep_[NHITS_MAX_];
  // Float_t hi_t_[NHITS_MAX_];
  // UShort_t hi_nhits_;
  std::vector<std::vector<Float_t > > x_;
  std::vector<std::vector<Float_t > > y_;
  std::vector<std::vector<Float_t > > z_;
  std::vector<std::vector<Float_t > > t_;
  std::vector<std::vector<Float_t>> de_;
  int clusterid_[1000];
  int ncluster_;

  // Short_t pi_stop_xstrip_;
  // Short_t pi_stop_ystrip_;
  // Short_t pi_stop_zlayer_;
  // Bool_t pi_stop_found_;

  bool debug_;
};

#endif
