/*
  Hit
 */
#ifndef __PI_ANA_HIT__
#define __PI_ANA_HIT__
#include "TROOT.h"

#include <Rtypes.h>
#include <set>
#include <tuple>
#include <vector>

#include "PIMCAtar.hh"

class PIAnaHitMerger;
class PIAnaHit;

class PIAnaG4StepDivider
{
public:
  PIAnaG4StepDivider(): step_limit_(0.005), g4_step_limit_(0.02) {}
  PIAnaG4StepDivider(const double step_limit) : step_limit_(step_limit) {}
 std::vector<PIAnaHit> process_atar_hit(PIMCAtar const &);
  double const step_limit() const { return step_limit_; }
  void step_limit(const double step_limit)
  { this->step_limit_ = step_limit; }
  double const g4_step_limit() const { return g4_step_limit_; }
  void g4_step_limit(const double g4_step_limit)
  { this->g4_step_limit_ = g4_step_limit; }

  ClassDef(PIAnaG4StepDivider, 1)
private:
  double step_limit_;
  double g4_step_limit_;
};

class PIAnaHitMerger
{
public:
  PIAnaHitMerger() {}
  PIAnaHitMerger(double const dt_min) : dt_min_(dt_min) {}
  double const dt_min() const { return dt_min_; }
  void dt_min(double const dt_min) { this->dt_min_ = dt_min; }
  bool merge(PIAnaHit const&, PIAnaHit const&);

  std::vector<PIAnaHit> merge(std::vector<PIAnaHit>);

  ClassDef(PIAnaHitMerger, 1)

private:
  // void merge(PIAnaHit&, PIAnaHit&);
  double dt_min_;
};

class PIAnaHit
{
public:
  PIAnaHit(): PIAnaHit(0, 0, -1E9,
                       0, 0, -1E9,
                       -1, -1E9, -1E9, -1,
                       -1, -1, 0, -1, false) {}
  PIAnaHit(double const x, double const y, double const z,
           double const rec_x, double const rec_y, double const rec_z,
           double const edep, double const post_t, double const dt,
           int const xstrip, int const ystrip,
           int const layer, int const pdgid, int const trackid,
           bool const valid):
             x_(x), y_(y), z_(z),
             rec_x_(rec_x), rec_y_(rec_y), rec_z_(rec_z),
             edep_(edep), post_t_(post_t), dt_(dt),
             xstrip_(xstrip), ystrip_(ystrip), layer_(layer),
             pdgid_(pdgid), trackid_(trackid), valid_(valid)
  { pdgids_.insert(pdgid); trackids_.insert(trackid); };
//  ~PIAnaHit();
  double const x() const { return this->x_; }
  double const y() const { return this->y_; }
  double const z() const { return this->z_; }
  double const rec_x() const { return this->rec_x_; }
  double const rec_y() const { return this->rec_y_; }
  double const rec_z() const { return this->rec_z_; }

  double const edep() const { return this->edep_; }
  double const t() const { return this->post_t_ - this->dt_; }
  double const post_t() const { return this->post_t_; }
  double const dt() const { return this->dt_; }
  int    const xstrip() const { return this->xstrip_; }
  int    const ystrip() const { return this->ystrip_; }
  int    const layer() const { return this->layer_; }
  bool   const valid() const { return this->valid_; }

  void x(double const x) { this->x_ = x; }
  void y(double const y) { this->y_ = y; }
  void z(double const z) { this->z_ = z; }
  void rec_x(double const rec_x) { this->rec_x_ = rec_x; }
  void rec_y(double const rec_y) { this->rec_y_ = rec_y; }
  void rec_z(double const rec_z) { this->rec_z_ = rec_z; }
  void edep(double const edep) { this->edep_ = edep; }
  void post_t(double const t) { this->post_t_ = t; }
  void dt(double const dt) { this->dt_ = dt; }
  void xstrip(int const xstrip) { this->xstrip_ = xstrip; }
  void ystrip(int const ystrip) { this->ystrip_ = ystrip; }
  void layer(int const layer) { this->layer_ = layer; }
  void valid(bool const valid) { this->valid_ = valid; }

  std::set<int> const& pdgids() const { return this->pdgids_; }
  void pdgids(std::set<int> const & pdgids)
  { this->pdgids_ = pdgids; }
  bool insert_pdgid(int const id)
  { return this->pdgids_.insert(id).second;}
  template<class InputIt> void insert_pdgid(InputIt first, InputIt last)
  { this->pdgids_.insert(first, last); }
  int const pdgid() const { return this->pdgid_; }
  void pdgid(int const id) { this->pdgid_ = id; }

  std::set<int> const& trackids() const { return this->trackids_; }
  void trackids(std::set<int> const & trackids)
  { this->trackids_ = trackids; }
  bool insert_trackid(int const id)
  { return this->trackids_.insert(id).second;}
  int const trackid() const { return this->trackid_; }
  void trackid(int const id) { this->trackid_ = id; }
  template<class InputIt> void insert_trackid(InputIt first, InputIt last)
  { this->trackids_.insert(first, last); }

  static std::pair<int, double> find_layer(double const);
  static std::pair<int, double> find_strip(double const);

  ClassDef(PIAnaHit, 1)

private:
  std::set<int> pdgids_;
  std::set<int> trackids_;
  double x_;
  double y_;
  double z_;
  double rec_x_;
  double rec_y_;
  double rec_z_;
  double post_t_;
  double dt_;
  double edep_;
  int    xstrip_;
  int    ystrip_;
  int    layer_;
  int    pdgid_;
  int    trackid_;
  bool   valid_;
};

#endif
