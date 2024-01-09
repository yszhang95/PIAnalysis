#ifndef __PI_ANA_PAT__
#define __PI_ANA_PAT__

#include "PIAnaHit.hpp"

#include "TH1F.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"

#include <Math/Functor.h>
#include <Math/Vector3D.h>

#include "Fit/Fitter.h"
#include "Fit/FitResult.h"

#include <TPolyLine3D.h>
#include <map>
#include <functional>


class TGraph2DErrors;

class PIAnaLineFitter
{
public:
  class PIAnaLineFunctor
  {
  public:
    PIAnaLineFunctor() {}
    void line(double t, const double* p, double &x, double &y, double &z);
    void load_data(TGraph2DErrors* g);
    double compute_pull2(double x, double y, double z,
                         double xerr, double yerr, double zerr,
                         const double*) const;
    double operator()(const double* p);
    bool const initialized() const { return (bool)g_; }
    TGraph2DErrors const& get_data();

  private:
    std::shared_ptr<TGraph2DErrors> g_;
  };

  PIAnaLineFitter() { for (int i=0; i!=4; ++i) { pars_[i]=0; } }
  void load_data(std::vector<PIAnaHit const *> hits,
                 const bool update_pars=true);
  bool fit();

  double compute_pull2(double x, double y, double z,
                       double xerr, double yerr, double zerr) const;

  TFitResultPtr const get_fit_result() const { return result_ptr_; }
  TPolyLine3D const* graphics() const;

private:

  ROOT::Fit::Fitter fitter_;
  TFitResultPtr result_ptr_;
  PIAnaLineFunctor func_;
  double pars_[4];
  std::unique_ptr<TPolyLine3D> line_;
};

class PIAnaLocCluster
{
public:
//  typedef std::reference_wrapper<const PIAnaHit> PIAnaHitRef;
  PIAnaLocCluster() : PIAnaLocCluster(false) {}
  PIAnaLocCluster(const bool verbose):
    pi_stop_x_(-1E3), pi_stop_y_(-1E3), pi_stop_z_(-1E3),
    pi_stop_t_(-1E3),
    pi_stop_xstrip_(-1), pi_stop_ystrip_(-1), pi_stop_zlayer_(-1),
    verbose_(verbose) {}
  ~PIAnaLocCluster()
  {
    pi_hits_.clear();
    mu_hits_.clear();
    e_hits_.clear();
  }

  std::pair<bool, PIAnaHit const *>
  get_pi_stop_hit(std::vector<std::vector<const PIAnaHit *>> const &);

  void cluster_hits(std::vector<std::vector<const PIAnaHit *>> const &);
  bool cluster_pi_hits(std::vector<std::vector<const PIAnaHit* > > const&);
  bool cluster_e_hits(std::vector<std::vector<const PIAnaHit *>> const &);

  std::pair<bool, PIAnaHit const *>
  get_pi_stop_hit(std::vector<const PIAnaHit* > const&);
  bool cluster_hits(std::vector<const PIAnaHit *> const &);
  const std::vector<const PIAnaHit *> &prompt_cluster() const {
    return p_hits_;
  }
  const std::vector<const PIAnaHit *> &nonprompt_cluster() const
  { return np_hits_; }

  void verbose(const bool verbose) { this->verbose_ = verbose; }
  void t0(const double t0) { this->t0_ = t0; };

  PIAnaLineFitter const& pi_fitter() const { return pi_fitter_; }
  PIAnaLineFitter const& e_fitter() const { return e_fitter_; }
  const bool verbose() const { return verbose_; }
  const double t0() const { return t0_; }

  ClassDef(PIAnaLocCluster, 1)

private:
  void setup_pi_stop(PIAnaHit const&);

  std::vector<PIAnaHit const*> p_hits_; // prompt hits
  std::vector<PIAnaHit const *> np_hits_; // nonprompt hits

  std::vector<PIAnaHit const*> pi_hits_;
  std::vector<PIAnaHit const*> mu_hits_;
  std::vector<PIAnaHit const*> e_hits_;

  PIAnaLineFitter pi_fitter_;
  PIAnaLineFitter e_fitter_;

  double pi_stop_x_;
  double pi_stop_y_;
  double pi_stop_z_;
  double pi_stop_t_;

  double t0_;

  int pi_stop_xstrip_;
  int pi_stop_ystrip_;
  int pi_stop_zlayer_;

  bool   verbose_;
};

class PIAnaPat
{
public:
  PIAnaPat(): verbose_(false) {};
  template <typename Iter>
  PIAnaPat(Iter first, Iter last) : hits_(first, last)
  { initialize_shared_loc(); }
  // input is a collection of hits in an event
  template <typename Iter>
  int process_event(Iter first, Iter last)
  {
    hits_.assign(first, last);
    return process_event();
  }
  int process_event(std::vector<PIAnaHit> const &);

  void verbose(const bool verbose) { this->verbose_ = verbose; }
  void t0(const double t0) { this->t0_ = t0; };

  PIAnaLocCluster const* local_cluster() { return colls_.get(); }

  const double t0() const { return t0_; }
  const bool verbose() const { return verbose_; }

  ClassDef(PIAnaPat, 1)

private:
  int process_event();
  void initialize_shared_loc();
  std::unique_ptr<PIAnaLocCluster> colls_;
  std::vector<std::vector<PIAnaHit const* > > shared_loc_;
  std::vector<PIAnaHit> hits_;

  std::unique_ptr<TH1F> hpass_;
  std::unique_ptr<TH1F> hall_;

  double t0_;
  bool verbose_;
};

#endif
