#include <iomanip>

#include "PIAnaPointCloud.hpp"
#include "PIPointCloud.hpp"
#include "nanoflann.hpp"

PIAnaPointCloud::PIAnaPointCloud() { index = nullptr; }

PIAnaPointCloud::~PIAnaPointCloud() {
  if (index)
    delete index;
  map_hit_indices_.clear();
  cloud_.pts.clear();
}

// https://github.com/BNLIF/wire-cell-data/blob/5c9fbc4aef81c32b686f7c2dc7b0b9f4593f5f9d/src/ToyPointCloud.cxx#L225
void PIAnaPointCloud::AddPoint(const PIAnaHit *hit)
{
  PIPointCloud<double, PIAnaHit>::Point point;
  // location
  point.x = hit->rec_x();
  point.y = hit->rec_y();
  point.z = hit->rec_z();
  // not sure what xyz indices are
  point.index_x = -1;
  point.index_y = -1;
  point.index_z = -1;
  // index
  point.index = cloud_.pts.size();
  // hit
  point.hit = hit;
  cloud_.pts.push_back(point);

  auto it = map_hit_indices_.find(hit);
  if (it == map_hit_indices_.end()) {
    map_hit_indices_[hit] = {};
  }
}

// https://github.com/BNLIF/wire-cell-data/blob/5c9fbc4aef81c32b686f7c2dc7b0b9f4593f5f9d/src/ToyPointCloud.cxx#L338
void PIAnaPointCloud::build_kdtree_index()
{
  if (index){
    delete index;
  }
  index = new my_kd_tree_t(
      3 /*dim*/, cloud_,
      nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */));
  index->buildIndex();
}

std::vector<nanoflann::ResultItem<size_t, double>>
PIAnaPointCloud::get_closest_index(Point &p, int N)

{
  std::vector<size_t> ret_index(N);
  std::vector<double> out_dist_sqr(N);

  double query_pt[3];
  query_pt[0] = p.x;
  query_pt[1] = p.y;
  query_pt[2] = p.z;

  N = index->knnSearch(&query_pt[0], N, &ret_index[0], &out_dist_sqr[0]);
  ret_index.resize(N);
  out_dist_sqr.resize(N);

  std::vector<nanoflann::ResultItem<size_t,double > > results(N);
  for(size_t i=0; i!=N; i++){
    results.at(i) =
      nanoflann::ResultItem{ret_index.at(i), out_dist_sqr.at(i)};
  }

  return results;
}

std::vector<nanoflann::ResultItem<size_t, double>>
    PIAnaPointCloud::get_closest_index(Point& p, double search_radius)
{
  double query_pt[3];
  query_pt[0] = p.x;
  query_pt[1] = p.y;
  query_pt[2] = p.z;
  std::vector < nanoflann::ResultItem <
    size_t, double>>
    ret_matches;
  nanoflann::SearchParameters params;
  const size_t nMatches =
      index->radiusSearch(&query_pt[0], search_radius * search_radius,
                          ret_matches, params);
  return ret_matches;
}

std::map<const PIAnaHit *, std::vector<int>>
PIAnaPointCloud::get_hit_indices_map(const double radius)
{
  for (auto it = map_hit_indices_.begin();
       it != map_hit_indices_.end(); ++it) {
    it->second.clear();
    Point p{it->first->rec_x(), it->first->rec_y(), it->first->rec_z()};
    auto indices = get_closest_index(p, radius);
    for (const auto &index : indices) {
      it->second.push_back(index.first);
    }
  }
  return map_hit_indices_;
}


std::ostream& operator<<(std::ostream &os, const PIAnaPointCloud &cloud)
{

  os << "[INFO] Printing out cloud content:\n";
  auto prec = os.precision(6);
  for (size_t i = 0; i != cloud.cloud_.pts.size(); ++i) {
    const auto &pt = cloud.cloud_.pts.at(i);
    if (i < 1000) {
      os << "[INFO] \t" << std::setw(3) << i << "th "
         << "Point (x, y, z) = (" << pt.x << ", " << pt.y << ", " << pt.z
         << "), index = " << pt.index << ", address of hit = " << pt.hit
         << ",\n";
      os << "[INFO] \t\t";
      auto it = cloud.map_hit_indices_.find(pt.hit);
      if (it == cloud.map_hit_indices_.end()) {
        os << "NOT FOUND the hit in the map.";
      } else {
        os << "FOUND the hit in the map. Its associated indices is (";
        std::vector<int> indices = it->second;
        if (indices.empty()) { os << ")."; }
        for (int i = 0; i < indices.size(); ++i) {
          if (i != indices.size() - 1) {
            os << indices.at(i) << ",";
          } else {
            os << indices.at(i) << ").";
          }
        }
      }
      os << "\n";

    } else {
      os << "[INFO] Ignoring hits with index > 1000.\n";
    }
  }
  os.precision(prec);
  return os;
}
