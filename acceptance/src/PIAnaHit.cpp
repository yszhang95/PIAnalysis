#include <TPolyLine3D.h>
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

  if (nsteps > 1000 && dr < g4_step_limit_) {
    // throw std::logic_error("nsteps > 1000");
    std::cerr << "[WARNING] nsteps > 1000\n";
  }

  const double edep = hit.GetEdep() / nsteps;
  const double dt = hit.GetDT() / nsteps;

  for (unsigned int istep=0; istep<nsteps; ++istep) {
    double x0 = pre_x + dx/nsteps * istep;
    double y0 = pre_y + dy/nsteps * istep;
    double z0 = pre_z + dz/nsteps * istep;
    double t1 = pos_t + dt        * istep;

    const auto xstrip  = PIAnaHit::find_strip( x0 );
    const auto ystrip  = PIAnaHit::find_strip( y0 );
    const auto layerid = PIAnaHit::find_layer( z0 );

    hits.emplace_back(x0, y0, z0,
                      xstrip.second, ystrip.second, layerid.second,
                      edep, t1, dt,
                      xstrip.first, ystrip.first, layerid.first,
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
  return merge;
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
      // z ordered; merge if layer == layer
      if (!merge_hits && hits.at(i).layer() < hits.at(j).layer()) {
        break;
      } else if (merge_hits) {
        auto& main_hit = hits.at(i).t() < hits.at(j).t() ? hits.at(i) : hits.at(j);
        auto& second_hit = hits.at(i).t() >= hits.at(j).t() ? hits.at(i) : hits.at(j);

        main_hit.edep(hits.at(i).edep() + hits.at(j).edep());
        // dt != hit1.dt + hit2.dt because hit1.post_t > hit2.t is possible
        // t is calculated using post_t and dt;
        // it must be called before modifying post_t
        const double dt = second_hit.post_t() - main_hit.t();
        main_hit.dt(dt);
        main_hit.post_t(second_hit.post_t());
        main_hit.insert_pdgid(second_hit.pdgids().begin(), second_hit.pdgids().end());
        main_hit.insert_trackid(second_hit.trackids().begin(), second_hit.trackids().end());

        main_hit.valid(true);
        second_hit.valid(false);
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

std::pair<int, double> PIAnaHit::find_layer(double const z)
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
      return std::make_pair(i, (z1+z0)/2.);
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
  return std::pair<int, double> {-1, -1E3};
}

std::pair<int, double> PIAnaHit::find_strip(double const loc)
{
  const int nstrips = 100;
  const double strip_w = 0.2;
  double loc0 = -1. * nstrips / 2. * strip_w; // mm
  double loc1 = loc0 + strip_w; // mm

  // optimize later
  for (int i=0; i<nstrips; ++i) {
    if ( loc < loc1 && loc >= loc0 ) {
      return std::pair<int, double>{i, (loc1+loc0)/2.};
    }
    loc0 += strip_w;
    loc1 += strip_w;
  }

  return std::pair<int, double> {-1, -1E5};
}


ClassImp(PIAnaG4StepDivider)
ClassImp(PIAnaHitMerger)
ClassImp(PIAnaHit)
