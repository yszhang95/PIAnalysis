#include "PIAnaPat.hpp"
#include <TDirectory.h>
#include <TPolyLine3D.h>
#include <memory>
#include <shared_mutex>

#include "Math/SVector.h"
#include "Math/SMatrix.h"
#include "Math/SMatrixDfwd.h"
#include "Fit/Fitter.h"
#include "Fit/FitResult.h"
#include "TFitResultPtr.h"
#include "TGraph2DErrors.h"
#include "TRandom.h"

void PIAnaLineFitter::PIAnaLineFunctor::load_data(TGraph2DErrors* g)
{
  g_.reset(g);
}

// define the parametric line equation
void PIAnaLineFitter::PIAnaLineFunctor::line
(double t, const double *p, double &x, double &y, double &z)
{
   // a parametric line is define from 6 parameters but 4 are independent
   // x0,y0,z0,z1,y1,z1 which are the coordinates of two points on the line
   // can choose z0 = 0 if line not parallel to x-y plane and z1 = 1;
   x = p[0] + p[1]*t;
   y = p[2] + p[3]*t;
   z = t;
}

double PIAnaLineFitter::PIAnaLineFunctor::compute_pull2
(double x, double y, double z,
 double xerr, double yerr, double zerr, const double* pars) const
{
  // distance line point is D= | (xp-x0) cross  ux |
  // where ux is direction of line and x0 is a point in the line (like t = 0)
  ROOT::Math::XYZVector xp(x,y,z);
  ROOT::Math::XYZVector x0(pars[0], pars[2], 0. );
  ROOT::Math::XYZVector x1(pars[0] + pars[1], pars[2] + pars[3], 1. );
  ROOT::Math::XYZVector u = (x1-x0).Unit();
  double d2 = ((xp-x0).Cross(u)).Mag2();

  const auto Y = u.Cross((xp-x0).Cross(u));
  //  ROOT::Math::Similarity();
  ROOT::Math::SVector<double, 3> vec(Y.X(), Y.Y(), Y.Z());
  ROOT::Math::SMatrixSym3D mat;
  mat(0,0) = xerr*xerr;
  mat(1,1) = yerr*yerr;
  mat(2,2) = zerr*zerr;
  double chi2 = d2*d2/4/ROOT::Math::Similarity(mat, vec);

  return chi2;
}

TGraph2DErrors const&
PIAnaLineFitter::PIAnaLineFunctor::get_data()
{
  return *g_;
}


double PIAnaLineFitter::PIAnaLineFunctor::operator() (const double* p)
{
  assert(g_);
  double* x = g_->GetX();
  double* y = g_->GetY();
  double* z = g_->GetZ();
  double* ex = g_->GetEX();
  double* ey = g_->GetEY();
  double* ez = g_->GetEZ();
  int np = g_->GetN();

  double chi2 = 0;
  for (int i=0; i<np; ++i) {
    double pull2 = compute_pull2(x[i], y[i], z[i],
                                ex[i], ey[i], ez[i], p);
    chi2 += pull2;
  }
  return chi2;
}

TPolyLine3D const* PIAnaLineFitter::graphics() const
{
  return new TPolyLine3D(*line_);
}

void PIAnaLineFitter::load_data(std::vector<PIAnaHit const *> hits,
                                const bool update_pars)
{
  if (hits.empty()) return;
  std::sort(std::begin(hits), std::end(hits),
            [](PIAnaHit const *h1, PIAnaHit const* h2) { return h1->layer() < h2->layer(); });
  TGraph2DErrors* g = new TGraph2DErrors(hits.size());
  g->SetDirectory(nullptr);
  std::string name(20, 0);
  std::generate_n(name.begin(), 20,
                  [](){ return gRandom->Integer(20);});
  g->SetName(name.c_str());
  for (std::vector<PIAnaHit const*>::size_type ip=0;
       ip!=hits.size(); ++ip) {
    const auto hit = hits.at(ip) ;
    g->SetPoint(ip, hit->rec_x(), hit->rec_y(), hit->rec_z());
    g->SetPointError(ip, hit->rec_xerr(), hit->rec_yerr(),
                     hit->rec_zerr());
  }
  func_.load_data(g);

  if (update_pars) {
      ROOT::Math::SMatrix2D mat;
      mat(0,0) = 1;
      mat(0,1) = hits.front()->rec_z();
      mat(1,0) = 1;
      mat(1,1) = hits.back()->rec_z();
      mat.InvertFast();

      ROOT::Math::SVector<double, 2> xvec{hits.front()->rec_x(), hits.back()->rec_x()};
      auto xpars = mat * xvec;
      pars_[0] = xpars(0);
      pars_[1] = xpars(1);

      ROOT::Math::SVector<double, 2> yvec{
        hits.front()->rec_y(), hits.back()->rec_y()};
      auto ypars = mat * yvec;
      pars_[2] = ypars(0);
      pars_[3] = ypars(1);
  }

}

bool PIAnaLineFitter::fit()
{
  if (!func_.initialized()) {
    return false;
  }
  const uint printlevel =
    ROOT::Math::MinimizerOptions::DefaultPrintLevel();
  const uint ncalls =
    ROOT::Math::MinimizerOptions::DefaultMaxFunctionCalls();
  const double prec = ROOT::Math::MinimizerOptions::DefaultPrecision();
  ROOT::Math::Functor fc(func_, 4);
  fitter_.SetFCN(fc, pars_);

  bool ok = fitter_.FitFCN();

  result_ptr_ = std::make_shared<TFitResult>(fitter_.Result());
  //  result_ptr_->Print();

  while (!ok) {
    double curr_prec = ROOT::Math::MinimizerOptions::DefaultPrecision();
    if (curr_prec < 0) curr_prec = 1E-6;
    //    ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(1);
    ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(3000);
    ROOT::Math::MinimizerOptions::SetDefaultPrecision(5 * curr_prec);
    ok = fitter_.FitFCN();
    result_ptr_ = std::make_shared<TFitResult>(fitter_.Result());

    if (curr_prec> 10) break;
  }

  if (!ok) {
    ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(printlevel);
    ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(ncalls);
    ROOT::Math::MinimizerOptions::SetDefaultPrecision(prec);
    Error("PIAnaLineFitter::fit", "Straight line fit failed.");
    return false;
  }

  line_ = std::make_unique<TPolyLine3D>(1000);
  auto g = func_.get_data();
  const double zmin = *std::min_element(g.GetZ(),
                                        g.GetZ()+g.GetN());
  const double zmax = *std::max_element(g.GetZ(),
                                        g.GetZ()+g.GetN());

  const double z0 = zmin - 0.02 * (zmax - zmin);
  const double z1 = zmax + 0.02 * (zmax - zmin);

  for (int i=0; i<1000; i++) {
    const double t = (z1-z0)/1000. * i + z0;
    double x=0, y=0, z=0;
    func_.line(t, pars_, x, y, z);
    line_->SetPoint(i, x, y, z);
  }

  const double* pars = result_ptr_->GetParams();
  for (int i=0; i<4; ++i) {
    pars_[i] = pars[i];
  }

  return true;
}

double
PIAnaLineFitter::compute_pull2
(double x, double y, double z, double xerr, double yerr, double zerr) const
{
  return func_.compute_pull2(x, y, z, xerr, yerr, zerr, pars_);
}


std::pair<bool, PIAnaHit const*>
PIAnaLocCluster::get_pi_stop_hit
(std::vector<std::vector<const PIAnaHit* > > const& shared_locs)
{
  if (verbose_) {
    std::cout << "[INFO] Dumping Input hits\n";
    for (const auto& loc : shared_locs) {
      std::cout << "x: " << loc.front()->xstrip()
      << "\ty: " << loc.front()->ystrip()
      << "\tz: " << loc.front()->layer();

      if (loc.size()>1) {
        for (const auto& l : loc) {
          std::cout << "\tt: " << l->t();
        }
      }
      std::cout << "\tnhits: " << loc.size() << "\n";
    }
  }

  std::vector<std::vector<PIAnaHit const* > > colls;
  for (const auto& loc : shared_locs) {
    if (loc.size()>1) {
      colls.push_back(loc);
    }
  }

  // find hit with smallest z when there is only one position shared by
  // multiple hits, otherwise find the largest z;
  if (colls.size() == 1) {
    setup_pi_stop(*colls.front().front());
    if (verbose_) {
      std::cout
      << "[INFO] Only one location at (xstrip, ystrip, zlayer) = ("
      << colls.front().front()->xstrip() << ","
      << colls.front().front()->ystrip() << ","
      << colls.front().front()->layer()
      << ") is shared by multiple hits\n";
    }
    return {true, colls.front().front()};
  } else if (colls.size() > 1) {
    // largest z must has adjacent hits with shared locations
    int nadjacent = colls.size() > 2 ? 2 : colls.size();
    for (auto h=colls.crbegin(); h!=colls.crend(); ++h) {
      auto h2 = std::next(h);
      bool adjacent = h->front()->layer() - h2->front()->layer() <=1;
      if (!adjacent) {
        std::cout << "[ERROR] Failed identifying pion location."
                     " Cannot find consecutive hits "
                     "with shared location.\n";
        return {false, nullptr};
      }
      if (--nadjacent <= 0) {
        break;
      }
    }
    if (verbose_) {
      std::cout << "[INFO] Found more than one shared locations.\n";
      std::cout << "[INFO] The one with largest z is chosen."
                   " (xstrip, ystrip, zlayer) = ("
            << colls.back().front()->xstrip() << ","
      << colls.back().front()->ystrip() << ","
      << colls.back().front()->layer() << ").\n";
    }

    setup_pi_stop(*colls.back().front());

    return {true, colls.back().front()};
  } else {
    std::cout << "[ERROR] Failed identifying pion location.\n";
  }
  return {false, nullptr};
}

void PIAnaLocCluster::setup_pi_stop(const PIAnaHit& hit)
{
  pi_stop_x_ = hit.rec_x();
  pi_stop_y_ = hit.rec_y();
  pi_stop_z_ = hit.rec_z();
  pi_stop_t_ = hit.t();

  pi_stop_xstrip_ = hit.xstrip();
  pi_stop_ystrip_ = hit.ystrip();
  pi_stop_zlayer_ = hit.layer();
}

void PIAnaLocCluster::cluster_hits
(std::vector<std::vector<const PIAnaHit* > > const& shared_locs)
{
  if (shared_locs.empty()) return;
  const auto pi_stop = get_pi_stop_hit(shared_locs);
  if (verbose_) {
    std::cout << "[INFO] PION Stopping location (x, y, z, t) = ("
    << pi_stop_x_ << "," << pi_stop_y_ << "," << pi_stop_z_
    << "," << pi_stop_t_ <<").\n";
  }

  cluster_pi_hits(shared_locs);
  cluster_e_hits(shared_locs);

  if (verbose_) {
    for (const auto hit : pi_hits_) {
      bool found = false;
      for (const auto pdgid : hit->pdgids()) {
        found = pdgid == 211 || found;
      }

      if (!found) std::cout << "Wrong association of pion hits.\n";
    }

    for (const auto hit : e_hits_) {
      bool found = false;
      for (const auto pdgid : hit->pdgids()) {
        found = pdgid == -11 || found;
      }
      if (!found) std::cout << "Wrong association of positron hits.\n";
    }

  }
}

bool PIAnaLocCluster::cluster_pi_hits
(std::vector<std::vector<const PIAnaHit* > > const& shared_locs)
{
  for (const auto& hits : shared_locs) {
    for (const auto hit : hits) {
      if (hit->t() < pi_stop_t_) {
        pi_hits_.push_back(hit);
      }
    }
  }

  pi_fitter_.load_data(pi_hits_);
  auto ok = pi_fitter_.fit();
  std::cout << "[INFO] Clustered " << pi_hits_.size()
  << " hits for the pion\n";

  return ok;
}

bool PIAnaLocCluster::cluster_e_hits
(std::vector<std::vector<const PIAnaHit* > > const& shared_locs)
{
  // first iteration
  for (const auto& hits : shared_locs) {
    for (const auto hit : hits) {
      if (hit->t() < pi_stop_t_) continue;

      const int dx = hit->xstrip() - pi_stop_xstrip_;
      const int dy = hit->ystrip() - pi_stop_ystrip_;
      const int dz = hit->layer() - pi_stop_zlayer_;

      const int dpixel2 = dx*dx + dy*dy + dz*dz;

      const bool adjacent = dpixel2 < 6*6 && dpixel2 >= 3*3;

      if (adjacent) {
        e_hits_.push_back(hit);
      }
    }
  }
  std::sort(e_hits_.begin(), e_hits_.end(),
            [](const PIAnaHit* h1, const PIAnaHit* h2)
              { return h1->t() < h2->t(); }
            );

  while(e_hits_.back()->t()-e_hits_.front()->t() > 1) {
    e_hits_.pop_back();
  }
  while (e_hits_.size()>5) {
    e_hits_.pop_back();
  }

  double chi2 = 100000.;
  while(chi2/e_hits_.size() > 4) {
    e_fitter_.load_data(e_hits_);
    auto ok = e_fitter_.fit();
    const auto result = e_fitter_.get_fit_result();
    chi2 = result->Chi2();
    std::vector<std::vector<PIAnaHit const* >::const_iterator > removed;
    for (auto it = e_hits_.cbegin(); it!= e_hits_.cend(); ++it) {
      const PIAnaHit& hit = *(*it);
      const auto pull2 =
        e_fitter_.compute_pull2(hit.rec_x(), hit.rec_y(), hit.rec_z(),
                                hit.rec_xerr(), hit.rec_yerr(), hit.rec_yerr());
      if (pull2 > 4) removed.push_back(it);
    }
    for (const auto it : removed) {
      e_hits_.erase(it);
    }
  }
  // second iteration
  for (const auto& hits : shared_locs) {
    for (const auto hit : hits) {
      if (std::abs(hit->t() - e_hits_.front()->t())>1) continue;

      const int dx = hit->xstrip() - pi_stop_xstrip_;
      const int dy = hit->ystrip() - pi_stop_ystrip_;
      const int dz = hit->layer() - pi_stop_zlayer_;

      const int dpixel2 = dx*dx + dy*dy + dz*dz;

      const bool adjacent = dpixel2 <= 3*3;

      if (adjacent) {
        e_hits_.push_back(hit);
      }
    }
  }

  std::sort(e_hits_.begin(), e_hits_.end(),
            [](const PIAnaHit* h1, const PIAnaHit* h2)
              { return h1->t() < h2->t(); }
            );

  chi2 = 10000;
  while(chi2/e_hits_.size() > 4) {
    e_fitter_.load_data(e_hits_);
    auto ok = e_fitter_.fit();
    const auto result = e_fitter_.get_fit_result();
    chi2 = result->Chi2();
    std::vector<std::vector<PIAnaHit const* >::const_iterator > removed;
    for (auto it = e_hits_.cbegin(); it!= e_hits_.cend(); ++it) {
      const PIAnaHit& hit = *(*it);
      const auto pull2 =
        e_fitter_.compute_pull2(hit.rec_x(), hit.rec_y(), hit.rec_z(),
                                hit.rec_xerr(), hit.rec_yerr(), hit.rec_yerr());
      if (pull2 > 4) removed.push_back(it);
    }
    for (const auto it : removed) {
      e_hits_.erase(it);
    }
  }

  std::cout << "[INFO] Clustered " << e_hits_.size()
  << " hits for the pion\n";
  return true;
}

bool
PIAnaLocCluster::cluster_hits(std::vector<const PIAnaHit*> const& hits)
{
  p_hits_.clear();
  np_hits_.clear();
  for (const auto hit : hits) {
    if (std::abs(hit->t() - t0_) < 1) {
      p_hits_.push_back(hit);
    } else
      if (std::abs(hit->t() - t0_) > 5) {
      np_hits_.push_back(hit);
    }
  }
  // temporarily skip the following
  return !np_hits_.empty();

  // iteratively add hits
  std::vector<PIAnaHit const *> pi_hits;
  std::vector<PIAnaHit const *>::const_iterator it_beg = p_hits_.cbegin();
  for (auto it = p_hits_.cbegin(); it != p_hits_.cend(); ++it) {
    const auto &hit = *it;
    if (pi_hits.empty()) {
      pi_hits.push_back(*it);
    } else if (std::abs(hit->xstrip() - pi_hits.front()->xstrip()) > 2 ||
               std::abs(hit->ystrip() - pi_hits.front()->ystrip()) > 2) {
      continue;
    } else {
      pi_hits.push_back(hit);
      it_beg = it;
    }
    if (pi_hits.size() > 3) {
      break;
    }
  }
  pi_fitter_.load_data(p_hits_);
  pi_fitter_.fit();
  for (auto it = it_beg; it != p_hits_.end(); ++it) {
    const auto& hit = *it;
    pi_hits.push_back(hit);

    pi_fitter_.load_data(pi_hits, false);
    const bool ok = pi_fitter_.fit();
    if (!ok) {
      pi_fitter_.load_data(pi_hits, true);
      bool pion = false;
      for (const auto pdgid : hit->pdgids()) {
        if (pdgid == 211) pion = true;
      }
      continue;
    }

    auto fit_result = pi_fitter_.get_fit_result();
    const double* pars = fit_result->GetParams();

    const auto pull2 = pi_fitter_.compute_pull2(
        hit->rec_x(), hit->rec_y(), hit->rec_z(), hit->rec_xerr(),
        hit->rec_yerr(), hit->rec_yerr());
    if (pull2 > 7 * 7)
      pi_hits.pop_back();

    // assume layer thickness is 120um
    // dx is in cm
    const auto dx = 0.012*std::sqrt(1 + pars[1]*pars[1] + pars[3]*pars[3]);
    const auto dedx = hit->edep()/dx;
    // 1 MIP ~ 3.875 MeV/cm
    const auto EMIP = 3.875;
    if (dedx < 3 * EMIP)
      pi_hits.pop_back();

    std::string msg = ::Form("dEdx: %.3f MeV/cm", dedx);
    Info("PIAnaLocCluster::cluster_hits", msg.c_str());
  }

  p_hits_ = pi_hits;

  return !np_hits_.empty();
}

// template<typename Iter>
// PIAnaPat::PIAnaPat(Iter first, Iter last)
int PIAnaPat::process_event(std::vector<PIAnaHit> const &rec_hits)
{
  hits_.assign(rec_hits.begin(), rec_hits.end());
  return process_event();
}

int PIAnaPat::process_event()
{
  std::vector<PIAnaHit const*> hit_ptrs;
  // initialize a list of pointers
  for(const auto& hit : hits_) {
    const PIAnaHit *h = &hit;
    hit_ptrs.push_back(h);
  }

  initialize_shared_loc();

  // first iteration
  colls_ = std::make_unique<PIAnaLocCluster>(verbose_);
  colls_->t0(t0_);
  // colls_->cluster_hits(shared_loc_);

  const bool has_np = colls_->cluster_hits(hit_ptrs);

  if (!has_np) {
    return 2;
  }
  return 0;
}


void PIAnaPat::initialize_shared_loc()
{
  shared_loc_.clear();
  if (hits_.empty()) {
    throw std::logic_error("[ERROR] PIAnaPat: Unnitialized hits.");
  }
  using sz = std::vector<PIAnaHit>::size_type;
  std::vector<bool> lock(hits_.size(), false);
  for (sz i=0; i<hits_.size(); ++i) {
    const auto& hit1 = hits_.at(i);
    std::vector<PIAnaHit const*> doublet;
    if (lock.at(i)) {
      continue;
    } else {
     doublet.push_back(&hit1);
     shared_loc_.push_back(doublet);
    }
    for (sz j=i+1; j<hits_.size(); ++j) {
      const auto& hit2 = hits_.at(j);
      // check if they share the same location
      if (hit1.layer() == hit2.layer()
          && hit1.xstrip() == hit2.xstrip()
          && hit1.ystrip() == hit2.ystrip()
          ) {
        // hits with same locations are associated to one vector
        lock.at(j) = true;
        // doublet is empty if the hit is locked
        if (!doublet.empty()) {
          shared_loc_.back().push_back(&hit2);
        }
      }
    }
  }

  auto orderbyz = [](std::vector<PIAnaHit const*>& c1,
                     std::vector<PIAnaHit const*>& c2)->bool
    { return c1.front()->layer() < c2.front()->layer();};
  std::sort(shared_loc_.begin(), shared_loc_.end(), orderbyz);

  auto orderbyt = [](PIAnaHit const* h1, PIAnaHit const* h2)  {
    return h1->t() < h2->t();
  };
  auto sortbyt = [&orderbyt](std::vector<PIAnaHit const*> v)
    {
      std::sort(v.begin(), v.end(), orderbyt); return v;
    };
  std::transform(shared_loc_.begin(), shared_loc_.end(),
                 shared_loc_.begin(), sortbyt);
}

ClassImp(PIAnaLocCluster)
ClassImp(PIAnaPat)
