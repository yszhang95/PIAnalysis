#ifndef __PI_ANA_PAT__
#define __PI_ANA_PAT__

#include "PIAnaHit.hpp"

#include "TH1F.h"

#include <map>
#include <functional>

class PIAnaLocCluster
{
public:
//  typedef std::reference_wrapper<const PIAnaHit> PIAnaHitRef;
  PIAnaLocCluster();
  PIAnaLocCluster(const bool verbose): verbose_(verbose) {};
  ~PIAnaLocCluster();

  std::pair<bool, PIAnaHit const*>
  get_pi_stop_hit(std::vector<std::vector<const PIAnaHit* > > const&);

  void cluster_hits(std::vector<std::vector<const PIAnaHit* > > const&);
  void verbose(const bool verbose) { this->verbose_ = verbose; }

  const bool verbose() const { return verbose_; }

  ClassDef(PIAnaLocCluster, 1)

private:
  std::vector<PIAnaHit const*> pi_hits_;
  std::vector<PIAnaHit const*> mu_hits_;
  std::vector<PIAnaHit const*> e_hits;

  double pi_stop_x_;
  double pi_stop_y_;
  double pi_stop_z_;

  bool   verbose_;
};

class PIAnaPat
{
public:
  PIAnaPat(): verbose_(false) {};
  template<typename Iter>
  PIAnaPat(Iter begin, Iter end);
  // input is a collection of hits in an event
  // template<typename Iter>
  // void process_event(Iter begin, Iter end);
  void process_event(std::vector<PIAnaHit> const&);

  void verbose(const bool verbose) { this->verbose_ = verbose; }

  const bool verbose() const { return verbose_; }

  ClassDef(PIAnaPat, 1)

private:

  void initialize_shared_loc();
  std::unique_ptr<PIAnaLocCluster> colls_;
  std::vector<std::vector<PIAnaHit const* > > shared_loc_;
  std::vector<PIAnaHit> hits_;

  std::unique_ptr<TH1F> hpass_;
  std::unique_ptr<TH1F> hall_;

  bool verbose_;
};

#endif
