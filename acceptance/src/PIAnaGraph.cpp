#include <boost/graph/connected_components.hpp>

#include "PIAnaHit.hpp"
#include "PIAnaPointCloud.hpp"
#include "PIAnaGraph.hpp"

PIAnaGraph::PIAnaGraph(const unsigned int dim) {
  if (dim == 3) {
    cloud_ = std::make_unique<PIAnaPointCloudXYZ>();
  } else if (dim == 1) {
    cloud_ = std::make_unique<PIAnaPointCloudT>();
  } else {
    std::string msg = "[ERROR] Cannot create a PIAnaGraph with dimension "
      + std::to_string(dim);
    throw std::logic_error(msg);
  }
  graph_ = std::make_unique<Graph>();
}

PIAnaGraph::~PIAnaGraph() {}

void PIAnaGraph::AddPoint(const PIAnaHit *hit) {
  cloud_->AddPoint(hit);
}

void PIAnaGraph::clear() {
  cloud_->clear();
  graph_.reset(new Graph());
}

std::map<int, PIAnaGraph::IndicesType>
PIAnaGraph::connected_components(const double radius) {
  std::map<int, IndicesType> results;
  graph_.reset(new Graph());
  cloud_->build_kdtree_index();
  auto map_hit_indices = cloud_->get_hit_indices_map(radius);
  for (const auto &pair_hit_indices : map_hit_indices) {
    if (pair_hit_indices.second.second.empty())
      continue;
    const auto hit = pair_hit_indices.first;
    const auto index = pair_hit_indices.second.first;
    for (const auto idx : pair_hit_indices.second.second) {
      if (idx != index) {
        boost::add_edge(index, idx, *graph_);
      }
    }
  }
  std::vector<int> component(boost::num_vertices(*graph_));
  int num = boost::connected_components(*graph_, &component[0]);
  for (int i = 0; i != num; ++i) {
    results[i] = IndicesType{};
  }
  for (IndexType i = 0; i != component.size(); ++i) {
    results.at(component.at(i)).push_back(i);
  }
  return results;
}
