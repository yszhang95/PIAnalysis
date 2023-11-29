#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "PIMCAtar.hh"
#include "PIAnaHit.hpp"

std::vector<PIAnaHit>
PIAnaG4StepDivider::process_atar_hit(PIMCAtar const & hit)
{

  std::vector<PIAnaHit> hits;

  const double pre_x = hit.GetX0();
  const double pre_y = hit.GetY0();
  const double pre_z = hit.GetZ0();
  const double pos_t = hit.GetTime();
  const double pos_x = hit.GetX1();
  const double pos_y = hit.GetY1();
  const double pos_z = hit.GetZ1();

  const double dx = pos_x - pre_x;
  const double dy = pos_y - pre_y;
  const double dz = pos_z - pre_z;

  double dr = std::sqrt(dx*dx + dy*dy + dz*dz);
  // double dr = hit.GetD();
  if (dr > g4_step_limit_) {
    std::cerr << "[ERROR] dr:" << dr << ", is too large for limit "
    << g4_step_limit_ << "\n"
    << "[ERROR] Check the G4 step size or set another limit\n"
    << "[ERROR] Empty collection of hits is returned.\n";
    return hits;
  }

  unsigned int nsteps =
    static_cast<unsigned int>(std::ceil(dr/this->step_limit_));

  if (nsteps > 100 && dr < g4_step_limit_) {
    throw std::logic_error("nsteps > 100");
  }

  const double step_size = dr / nsteps;
  const double edep = hit.GetEdep() / nsteps;
  const double dt = hit.GetDT() / nsteps;

  for (unsigned int istep=0; istep<nsteps; ++istep) {
    double x0 = pre_x + step_size * istep;
    double y0 = pre_y + step_size * istep;
    double z0 = pre_z + step_size * istep;
    double t1 = pos_t + dt        * istep;

    const int xstrip  = PIAnaHit::find_strip( x0 );
    const int ystrip  = PIAnaHit::find_strip( y0 );
    const int layerid = PIAnaHit::find_layer( z0 );

    hits.emplace_back(x0, y0, z0, edep, t1, dt, xstrip, ystrip, layerid,
                      hit.GetPDGID(), hit.GetTrackID(), true);
  }
  return hits;
}

bool PIAnaHitMerger::merge(PIAnaHit const& h1, PIAnaHit const& h2)
{
  bool merge = std::abs(h1.t() - h2.t()) < dt_min_;
  merge = merge && h1.xstrip() == h2.xstrip();
  merge = merge && h1.ystrip() == h2.ystrip();
  merge = merge && h1.layer() == h2.layer();
  return true;
}

std::vector<PIAnaHit> PIAnaHitMerger::merge(std::vector<PIAnaHit> hits)
{
  std::sort(hits.begin(), hits.end(),
            [](PIAnaHit const& h1, PIAnaHit const& h2)->bool
              { return h1.layer() < h2.layer(); });

  for (std::vector<PIAnaHit>::size_type i=0; i<hits.size(); ++i) {
    if (!hits.at(i).valid()) continue;
    for (std::vector<PIAnaHit>::size_type j=i+1; j<hits.size(); ++j) {
      if (!hits.at(j).valid()) continue;
      bool merge_hits = merge(hits.at(i), hits.at(j));
      // z ordered
      if (!merge_hits && hits.at(i).layer() < hits.at(j).layer()) {
        break;
      } else if (merge_hits) {
        auto& main_hit = hits.at(i).t() < hits.at(j).t() ? hits.at(i) : hits.at(j);
        auto& second_hit = hits.at(i).t() >= hits.at(j).t() ? hits.at(i) : hits.at(j);

        main_hit.edep(hits.at(i).edep() + hits.at(j).edep());
        main_hit.post_t(second_hit.post_t());
        main_hit.dt(hits.at(i).dt() + hits.at(j).dt());
        main_hit.insert_pdgid(second_hit.pdgids().begin(), second_hit.pdgids().end());
        main_hit.insert_trackid(second_hit.trackids().begin(), second_hit.trackids().end());

        main_hit.valid(false);
        second_hit.valid(true);
      }
    }
  }
  // https://stackoverflow.com/a/30616738
  hits.erase(std::remove_if(hits.begin(), hits.end(),
                            [](PIAnaHit const& hit) -> bool { return !hit.valid(); })
             , hits.end());
  std::sort(std::begin(hits), std::end(hits),
            [](PIAnaHit const& h1, PIAnaHit const& h2)->bool { return h1.t() < h2.t(); });
  return hits;
}

int PIAnaHit::find_layer(double const z)
{
  const int nlayers = 48;
  double thickness = 0.12; // mm
  const double elec_thick = 0.002; // mm
//  const double pass_thick = 0.002; // mm
  const double pass_thick = 0.0; // mm
  thickness += 2*elec_thick;
  // https://github.com/PIONEER-Experiment/geometry/blob/feature/doublesided/generator/atar.py#L591
  // ilayer % 2 == 0, + pass_thick
  double z0 = -thickness/2.; // mm
  double z1 = z0 + thickness; // mm
  z0 += pass_thick/2.;
  z1 += pass_thick/2.;
  // optimize later
  for (int i=0; i<nlayers; ++i) {
    if (z < z1 && z>= z0) {
      return i;
    }
    z0 += thickness;
    z1 += thickness;
    if (i%2==0) {
      z0 -= pass_thick/2.;
      z1 -= pass_thick/2.;
    } else {
      z0 += pass_thick/2.;
      z1 += pass_thick/2.;
    }
  }
  return -1;
}

int PIAnaHit::find_strip(double const loc)
{
  const int nstrips = 100;
  const double strip_w = 0.2;
  double loc0 = -1. * nstrips / 2. * strip_w; // mm
  double loc1 = loc0 + strip_w; // mm

  // optimize later
  for (int i=0; i<nstrips; ++i) {
    if ( loc < loc1 && loc > loc0 ) {
      return i;
    }
    loc0 += strip_w;
    loc1 += strip_w;
  }
  return -1;
}


ClassImp(PIAnaG4StepDivider)
ClassImp(PIAnaHitMerger)
ClassImp(PIAnaHit)
