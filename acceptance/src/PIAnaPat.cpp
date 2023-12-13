#include "PIAnaPat.hpp"
#include <memory>
#include <shared_mutex>

PIAnaLocCluster::PIAnaLocCluster()
: pi_stop_x_(-1E9), pi_stop_y_(-1E9), pi_stop_z_(-1E9)
{
}

PIAnaLocCluster::~PIAnaLocCluster()
{
  pi_hits_.clear();
  mu_hits_.clear();
  e_hits.clear();
}

// template<typename RandomIter, typename ContainerType>
// std::pair<bool, RandomIter>
// PIAnaLocCluster::get_pi_stop_hit(RandomIter first, RandomIter last)

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
    pi_stop_x_ = colls.front().front()->rec_x();
    pi_stop_y_ = colls.front().front()->rec_y();
    pi_stop_z_ = colls.front().front()->rec_z();
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

    pi_stop_x_ = colls.back().front()->rec_x();
    pi_stop_y_ = colls.back().front()->rec_y();
    pi_stop_z_ = colls.back().front()->rec_z();
    return {true, colls.back().front()};
  } else {
    std::cout << "[ERROR] Failed identifying pion location.\n";
  }
  return {false, nullptr};
}

// template<typename RandomIter>
// void PIAnaLocCluster::cluster_hits(RandomIter first, RandomIter last)
// {
//   if (first == last)
//     return;

//   const auto pi_stop = get_pi_stop_hit(first, last);

// }

void PIAnaLocCluster::cluster_hits
(std::vector<std::vector<const PIAnaHit* > > const& shared_locs)
{
  if (shared_locs.empty()) return;
  const auto pi_stop = get_pi_stop_hit(shared_locs);
  std::cout << "x: " << pi_stop_x_
  << "\ty: " << pi_stop_y_
  << "\tz: " << pi_stop_z_ << "\n";
}


template<typename Iter>
PIAnaPat::PIAnaPat(Iter first, Iter last)
: hits_(first, last)
{
  initialize_shared_loc();
}

// template<typename Iter>
// void PIAnaPat::process_event(Iter first, Iter last)

void PIAnaPat::process_event(std::vector<PIAnaHit> const& rec_hits)

{
  hits_.assign(rec_hits.begin(), rec_hits.end());
  initialize_shared_loc();

  // first iteration
  colls_ = std::make_unique<PIAnaLocCluster>(verbose_);
  colls_->cluster_hits(shared_loc_);
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
