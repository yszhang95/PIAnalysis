#include <algorithm>
#include <numeric>

#include "PIAnaCluster.hpp"

PITCluster::PITCluster() : t0_(0)
{
  this->PILocCluster::graph_ = std::make_unique<PIAnaGraph>(1);
}

PITCluster::~PITCluster() {}

std::map<const PIAnaHit*, std::vector<const PIAnaHit * > >
PITCluster::connected_hits(const std::vector<const PIAnaHit *> &hits)
{
  std::map<const PIAnaHit *, std::vector<const PIAnaHit *>> results;
  graph_->clear();
  for (const auto &hit : hits) {
    graph_->AddPoint(hit);
  }
  PIAnaPointCloud::Point p{t0_, 0, 0};
  // sorted by distance
  auto idxs = graph_->connected_components(p, radius_);
  // connected hits
  // sorted by distance, the first one is the closest
  const PIAnaHit *hitptr =
      idxs.empty() ? nullptr : graph_->get_hit(idxs.front());

  if (!idxs.empty() && !hitptr) {
    throw std::logic_error("PITCluster::connected_hits: the reference"
                           " point is connected to a nullptr.\n");
  }
  if (hitptr) {
    results[hitptr] = {};
    for (const auto idx : idxs) {
      results.at(hitptr).push_back(graph_->get_hit(idx));
    }
  }

  // unconnected hits
  results[nullptr] = {};

  // everything must be sorted
  for (size_t i = 0; i != graph_->get_num_points(); ++i) {
    const auto hit = graph_->get_hit(i);
    auto it = std::find_if(results.at(hitptr).begin(), results.at(hitptr).end(),
                           // compare addresses of pointers,
                           // not the objects which are pointed to
                           [&hit](const PIAnaHit* h) { return h == hit; });
    if (it == results.at(hitptr).end()) {
      results.at(nullptr).push_back(hit);
    }
  }
  std::sort(
      results.at(nullptr).begin(), results.at(nullptr).end(),
      [](const PIAnaHit *h1, const PIAnaHit *h2) { return h1->t() < h2->t(); });

  return results;
}

std::map<const PIAnaHit*, std::vector<const PIAnaHit * > >
PITCluster::cluster_all_hits(const std::vector<const PIAnaHit *> &allhits)
{
  std::map<const PIAnaHit *, std::vector<const PIAnaHit *>> results;
  const double init_t0 = t0_;
  for (std::map<const PIAnaHit *, std::vector<const PIAnaHit *>> output
         = connected_hits(allhits);
       ;output = connected_hits(output.at(nullptr))) {

    for (const auto &hit_assoc : output) {
      if (hit_assoc.first != nullptr) {
        auto it = results.insert(hit_assoc);
        if (!it.second) {
          throw std::logic_error("PITCluster::cluster_hits: found shared"
                                 " pointer to the same hit by two clusters."
                                 " Check the algorithm\n");
        }
      }
    }
    // output is sorted by time in connected_hits()
    t0_ = output.at(nullptr).empty() ? t0_ : output.at(nullptr).front()->t();

    if (output.at(nullptr).empty()) {break;}
  }
  t0_ = init_t0;

  return results;
}

PIXYZCluster::PIXYZCluster()
{
  this->PILocCluster::graph_ = std::make_unique<PIAnaGraph>(3);
}

PIXYZCluster::~PIXYZCluster() {}

std::map<const PIAnaHit*, std::vector<const PIAnaHit * > >
PIXYZCluster::connected_hits(const std::vector<const PIAnaHit *> &hits)
{
  std::map<const PIAnaHit *, std::vector<const PIAnaHit *>> results;
  graph_->clear();
  for (const auto &hit : hits) {
    graph_->AddPoint(hit);
  }
  // groups of cluster index, cluster of hits
  // sorted by distance in each group
  auto idxs = graph_->connected_components(radius_);
  for (const auto &hits : idxs) {
    if (hits.second.empty())
      continue;
    const auto firsthit = graph_->get_hit(hits.second.front());
    results.insert({firsthit, {}});
    for (const auto hitidx : hits.second) {
      const auto hit = graph_->get_hit(hitidx);
      results.at(firsthit).push_back(hit);
    }
  }
  return results;
}

std::map<const PIAnaHit*, std::vector<const PIAnaHit * > >
PIXYZCluster::cluster_all_hits(const std::vector<const PIAnaHit *> &allhits)
{
  auto results = connected_hits(allhits);
  return results;
}
