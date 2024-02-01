#include <iomanip>
#include <stdexcept>

#include "PIAnaPointCloud.hpp"
#include "PIPointCloud.hpp"
#include "nanoflann.hpp"

PIAnaPointCloud::PIAnaPointCloud() { }

PIAnaPointCloud::~PIAnaPointCloud() {
  map_hit_indices_.clear();
  cloud_.pts.clear();
}

void PIAnaPointCloud::clear() {
  map_hit_indices_.clear();
  cloud_.pts.clear();
}

void PIAnaPointCloud::AddPoint(const Point& p, const PIAnaHit* hit)
{
  auto it = map_hit_indices_.find(hit);
  if (it != map_hit_indices_.end()) {
    return;
  }

  PIPointCloud<double, PIAnaHit>::Point point;
  // location
  point.x = p.x;
  point.y = p.y;
  point.z = p.z;
  // not sure what xyz indices are
  point.index_x = -1;
  point.index_y = -1;
  point.index_z = -1;
  // index
  point.index = cloud_.pts.size();
  // hit
  point.hit = hit;
  cloud_.pts.push_back(point);

  map_hit_indices_[hit] = std::make_pair(point.index, IndicesType{});
}

// https://isocpp.org/wiki/faq/strange-inheritance#calling-virtuals-from-base
std::map < const PIAnaHit *, std::pair < PIAnaPointCloud::IndexType,
                                         PIAnaPointCloud::IndicesType > >
PIAnaPointCloud::get_hit_indices_map(const double radius)
{
  for (auto it = map_hit_indices_.begin();
       it != map_hit_indices_.end(); ++it) {
    it->second.second.clear();
    Point p = get_point(it->first);
    auto indices = get_closest_index(p, radius);
    for (const auto &index : indices) {
      it->second.second.push_back(index.first);
    }
  }
  return map_hit_indices_;
}

const PIAnaHit *PIAnaPointCloud::get_hit(const IndexType idx) const {
  return cloud_.get_hit(idx);
}

std::ostream& operator<<(std::ostream &os, const PIAnaPointCloud &cloud)
{
  const PIAnaPointCloud* ptr = &cloud;
  os << "[INFO] Printing out cloud content:\n";
  auto prec = os.precision(6);
  for (size_t i = 0; i != cloud.cloud_.pts.size(); ++i) {
    const auto &pt = cloud.cloud_.pts.at(i);
    if (i < 1000) {
      if (dynamic_cast<const PIAnaPointCloudXYZ*>(ptr)) {
        os << "[INFO] \t" << std::setw(3) << i << "th "
           << "Point (x, y, z) = (" << pt.x << ", " << pt.y << ", " << pt.z
           << "), index = " << pt.index << ", address of hit = " << pt.hit
           << ",\n";
      } else if (dynamic_cast<const PIAnaPointCloudT *>(ptr)) {
        os << "[INFO] \t" << std::setw(3) << i << "th "
           << "Point (t) = (" << pt.x
           << "), index = " << pt.index << ", address of hit = " << pt.hit
           << ",\n";
      }
      os << "[INFO] \t\t";
      auto it = cloud.map_hit_indices_.find(pt.hit);
      if (it == cloud.map_hit_indices_.end()) {
        os << "NOT FOUND the hit in the map.";
      } else {
        PIAnaPointCloud::IndexType index = it->second.first;
        PIAnaPointCloud::IndicesType indices = it->second.second;
        os << "FOUND the hit in the map. Its index is " << index
           << ". Its associated indices are (";
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

// PIAnaPointCloud3D
PIAnaPointCloud3D::PIAnaPointCloud3D() : index(nullptr) {}

PIAnaPointCloud3D::~PIAnaPointCloud3D() {
  if (index)
    delete index;
}

void PIAnaPointCloud3D::clear() {
  if (index) delete index;
  PIAnaPointCloud::clear();
}

// https://github.com/BNLIF/wire-cell-data/blob/5c9fbc4aef81c32b686f7c2dc7b0b9f4593f5f9d/src/ToyPointCloud.cxx#L338
void PIAnaPointCloud3D::build_kdtree_index()
{
  if (index){
    delete index;
  }
  index = new my_kd_tree_3d_t(
      3 /*dim*/, cloud_,
      nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */));
  index->buildIndex();
}

std::vector<nanoflann::ResultItem<PIAnaPointCloud::IndexType, double>>
PIAnaPointCloud3D::get_closest_index(Point &p, int N)
{
  IndicesType ret_index(N);
  std::vector<double> out_dist_sqr(N);

  double query_pt[3];
  query_pt[0] = p.x;
  query_pt[1] = p.y;
  query_pt[2] = p.z;

  N = index->knnSearch(&query_pt[0], N, &ret_index[0], &out_dist_sqr[0]);
  ret_index.resize(N);
  out_dist_sqr.resize(N);

  std::vector<nanoflann::ResultItem<IndexType,double > > results(N);
  for(size_t i=0; i!=N; i++){
    results.at(i) =
      nanoflann::ResultItem{ret_index.at(i), out_dist_sqr.at(i)};
  }

  return results;
}

std::vector<nanoflann::ResultItem<PIAnaPointCloud::IndexType, double>>
    PIAnaPointCloud3D::get_closest_index(Point& p, double search_radius)
{
  double query_pt[3];
  query_pt[0] = p.x;
  query_pt[1] = p.y;
  query_pt[2] = p.z;
  std::vector < nanoflann::ResultItem <
    PIAnaPointCloud::IndexType, double>>
    ret_matches;
  nanoflann::SearchParameters params;
  const size_t nMatches =
      index->radiusSearch(&query_pt[0], search_radius * search_radius,
                          ret_matches, params);
  return ret_matches;
}

// PIAnaPointCloudXYZ
PIAnaPointCloudXYZ::PIAnaPointCloudXYZ() {}
PIAnaPointCloudXYZ::~PIAnaPointCloudXYZ() {}

// https://github.com/BNLIF/wire-cell-data/blob/5c9fbc4aef81c32b686f7c2dc7b0b9f4593f5f9d/src/ToyPointCloud.cxx#L225
void
PIAnaPointCloudXYZ::AddPoint(const PIAnaHit *hit)
{
  Point point = this->get_point(hit);
  PIAnaPointCloud::AddPoint(point, hit);
}

PIAnaPointCloud::Point
PIAnaPointCloudXYZ::get_point(const PIAnaHit* hit)
{
  return Point{hit->rec_x(), hit->rec_y(), hit->rec_z()};
}

// PIAnaPointCloud1D
PIAnaPointCloud1D::PIAnaPointCloud1D() : index(nullptr) {}

PIAnaPointCloud1D::~PIAnaPointCloud1D() {
  if (index)
    delete index;
}

void PIAnaPointCloud1D::clear() {
  if (index) delete index;
  PIAnaPointCloud::clear();
}

// https://github.com/BNLIF/wire-cell-data/blob/5c9fbc4aef81c32b686f7c2dc7b0b9f4593f5f9d/src/ToyPointCloud.cxx#L338
void PIAnaPointCloud1D::build_kdtree_index()
{
  if (index) {
    delete index;
  }
  index = new my_kd_tree_1d_t(
      1 /*dim*/, cloud_,
      nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */));
  index->buildIndex();
}

std::vector<nanoflann::ResultItem<PIAnaPointCloud::IndexType, double>>
PIAnaPointCloud1D::get_closest_index(Point &p, int N)
{
  IndicesType ret_index(N);
  std::vector<double> out_dist_sqr(N);

  double query_pt[3];
  query_pt[0] = p.x;

  N = index->knnSearch(&query_pt[0], N, &ret_index[0], &out_dist_sqr[0]);
  ret_index.resize(N);
  out_dist_sqr.resize(N);

  std::vector<nanoflann::ResultItem<IndexType,double > > results(N);
  for(size_t i=0; i!=N; i++){
    results.at(i) =
      nanoflann::ResultItem{ret_index.at(i), out_dist_sqr.at(i)};
  }

  return results;
}

std::vector<nanoflann::ResultItem<PIAnaPointCloud::IndexType, double>>
    PIAnaPointCloud1D::get_closest_index(Point& p, double search_radius)
{
  double query_pt[1];
  query_pt[0] = p.x;
  std::vector < nanoflann::ResultItem <
    PIAnaPointCloud::IndexType, double>>
    ret_matches;
  nanoflann::SearchParameters params;
  const size_t nMatches =
      index->radiusSearch(&query_pt[0], search_radius * search_radius,
                          ret_matches, params);
  return ret_matches;
}

// PIAnaPointCloudT
PIAnaPointCloudT::PIAnaPointCloudT() {}
PIAnaPointCloudT::~PIAnaPointCloudT() {}

PIAnaPointCloud::Point
PIAnaPointCloudT::get_point(const PIAnaHit* hit) {
  return Point{hit->t(), 0, 0};
}

void
PIAnaPointCloudT::AddPoint(const PIAnaHit *hit)
{
  Point point = get_point(hit);
  PIAnaPointCloud::AddPoint(point, hit);
}
