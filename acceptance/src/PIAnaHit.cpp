#include <TPolyLine3D.h>
#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "PIMCAtar.hh"
#include "PIAnaHit.hpp"

bool operator<(const PIAnaHit &h1, const PIAnaHit &h2)
{
  if (h1.t() < h2.t())
    return true;
  else if (h1.t() == h2.t() && h1.rec_z() < h2.rec_z())
    return true;
  else if (h1.t() == h2.t() && h1.rec_z() == h2.rec_z() && h1.rec_x() < h2.rec_x())
    return true;
  else if (h1.t() == h2.t() && h1.rec_z() == h2.rec_z() &&
           h1.rec_x() == h2.rec_x() && h1.rec_y() == h2.rec_y())
    return true;
  return false;
}

std::vector<PIAnaHit>
PIAnaG4StepDivider::process_atar_hit(PIMCAtar const & hit)
{

  std::vector<PIAnaHit> hits;

  const double pre_t = hit.GetTime() - hit.GetDT();
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

  std::array<double, 4> dtxyz;
  dtxyz.at(0) = dt;
  dtxyz.at(1) = dx / nsteps;
  dtxyz.at(2) = dy / nsteps;
  dtxyz.at(3) = dz / nsteps;

  for (unsigned int istep = 0; istep < nsteps; ++istep) {
    std::array<double, 4> txyz0, txyz1, txyzmid;
    txyz0.at(0) = pre_t + dtxyz.at(0) * istep;
    txyz0.at(1) = pre_x + dtxyz.at(1) * istep;
    txyz0.at(2) = pre_y + dtxyz.at(2) * istep;
    txyz0.at(3) = pre_z + dtxyz.at(3) * istep;

    for (unsigned int i = 0; i != 4; ++i) {
      txyz1.at(i) = txyz0.at(i) + dtxyz.at(i);
      txyzmid.at(i) = txyz0.at(i) + dtxyz.at(i)/2.;
    }

    std::array<std::pair<int, double>, 3> pre_step, pos_step, mid_step;
    {
      for (unsigned int i = 0; i != 2; ++i) {
        pre_step.at(i) = PIAnaHit::find_strip(txyz0.at(i + 1));
        pos_step.at(i) = PIAnaHit::find_strip(txyz1.at(i + 1));
        mid_step.at(i) = PIAnaHit::find_strip(txyzmid.at(i+1));
      }
      pre_step.at(2) = PIAnaHit::find_layer(txyz0.at(3));
      pos_step.at(2) = PIAnaHit::find_layer(txyz1.at(3));
      mid_step.at(2) = PIAnaHit::find_layer(txyzmid.at(3));
    }

    // Prerequisites: step size must be smaller than pitch size and layer
    // thickness.
    // Three steps at most in advance means four steps at most in total.
    // Tetermine their order by step size.
    std::array<double, 5> fracs{0, 1, 1, 1, 1};
    unsigned int ncrossed = 0;
    for (unsigned int i = 0; i != 3; ++i) {
      const bool cross_component = pre_step.at(i).first != pos_step.at(i).first;
      if (cross_component) {
        // std::cout << "Printing out\n";
        // std::cout << "Crossed along " << i + 1
        //           << " component where i is from 1 to 3, at boundary "<<
        //   (pre_step.at(i).second + pos_step.at(i).second)/2. <<
        //   "\n";
        ncrossed++;
        const double dist =
            std::abs((pre_step.at(i).second + pos_step.at(i).second) / 2.
                     - txyz0.at(i + 1));
        fracs.at(i + 1) = dist / std::abs(dtxyz.at(i + 1));
        // std::cout << "Corresponding (distance, fraction, total step size) to "
        //              "boundary is ("
        //           << dist << ", " << fracs.at(i+1) << "," << dtxyz.at(i+1) << ")" << std::endl;
      }
    }
    std::sort(std::begin(fracs), std::end(fracs),
              [](const double i, const double j) { return i<j; });
    if (ncrossed) {
      // std::cout << "Crossed boundaries " << ncrossed
      //           << " times from (" << txyz0.at(0) << "," << txyz0.at(1) << "," << txyz0.at(2) << "," << txyz0.at(3) << ") to (" << txyz1.at(0) << "," << txyz1.at(1) << "," << txyz1.at(2) << "," << txyz1.at(3) <<
      //   ")\n";
      for (unsigned int i = 0; i != ncrossed+1; ++i) {
        const double dfrac = fracs.at(i + 1) - fracs.at(i);
        // t, x, y, z
        // std::cout << "Step forwad by a fraction of " << dfrac << "\n";
        for (unsigned int j = 0; j != 4; ++j) {
          txyz1.at(j) = txyz0.at(j) + dfrac * dtxyz.at(j);
          txyzmid.at(j) = txyz0.at(j) + dfrac*dtxyz.at(j)/2.;
        }
        // update values for this iteration
        for (unsigned int i = 0; i != 2; ++i) {
          pre_step.at(i) = PIAnaHit::find_strip(txyz0.at(i + 1));
          pos_step.at(i) = PIAnaHit::find_strip(txyz1.at(i + 1));
          mid_step.at(i) = PIAnaHit::find_strip(txyzmid.at(i+1));
        }
        pre_step.at(2) = PIAnaHit::find_layer(txyz0.at(3));
        pos_step.at(2) = PIAnaHit::find_layer(txyz1.at(3));
        mid_step.at(2) = PIAnaHit::find_layer(txyzmid.at(3));
        // insert new element
        if (mid_step.at(0).first >= 0 && mid_step.at(1).first >= 0 &&
            mid_step.at(2).first >= 0) {
          hits.emplace_back(txyzmid.at(1), txyzmid.at(2), txyzmid.at(3),
                            mid_step.at(0).second, mid_step.at(1).second,
                            mid_step.at(2).second, edep * dfrac, txyz1.at(0), dtxyz.at(0) * dfrac,
                            mid_step.at(0).first, mid_step.at(1).first, mid_step.at(2).first,
                            hit.GetPDGID(), hit.GetTrackID(), true);
        }

        // update values for next iteration
        for (unsigned int j = 0; j != 4; ++j) {
          txyz0.at(j) = txyz1.at(j);
        }
      }
    } else {
      if (mid_step.at(0).first >= 0 && mid_step.at(1).first >= 0 &&
          mid_step.at(2).first >= 0) {
        hits.emplace_back(txyzmid.at(1), txyzmid.at(2), txyzmid.at(3),
                          mid_step.at(0).second, mid_step.at(1).second,
                          mid_step.at(2).second, edep, txyz1.at(0),
                          dtxyz.at(0), mid_step.at(0).first,
                          mid_step.at(1).first, mid_step.at(2).first,
                          hit.GetPDGID(), hit.GetTrackID(), true);
      }
    }
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
