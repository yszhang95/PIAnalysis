#ifndef __PI__AnaPointCloud__
#define __PI__AnaPointCloud__

#include "nanoflann.hpp"
#include "PIAnaHit.hpp"
#include "PIPointCloud.hpp"

#include "Vector.h"

#include <cstddef>

/**
   There is a difference between WCP::WCPointCloud::Point and WCP::Point.
   The class WCP::Point is a vector, defined in
https://github.com/BNLIF/wire-cell-data/blob/5c9fbc4aef81c32b686f7c2dc7b0b9f4593f5f9d/inc/WCPData/Vector.h#L22
 */

class PIAnaPointCloud
{
public:
  typedef WCP::D3Vector<double> Point;

  typedef nanoflann::KDTreeSingleIndexAdaptor<
nanoflann::L2_Simple_Adaptor<double, PIPointCloud<double, PIAnaHit> > ,
  PIPointCloud<double, PIAnaHit>,
  3 /* dim */, size_t
  > my_kd_tree_t;

  PIAnaPointCloud();
  ~PIAnaPointCloud();

  friend std::ostream &operator<<(std::ostream &os,
                                  const PIAnaPointCloud&);

  void AddPoint(const PIAnaHit *hit);
  void build_kdtree_index();

  // std::map<PIAnaHit *, PIPointCloud<double, PIAnaHit>::Point>
  // get_closest_pixel(PIPointCloud<double, PIAnaHit>::Point, int N);
  // std::map<PIAnaHit *, PIPointCloud<double, PIAnaHit>::Point>
  // get_closest_pixel(PIPointCloud<double, PIAnaHit>::Point, double radius);

  size_t get_num_points() { return cloud_.pts.size(); }

  std::vector<nanoflann::ResultItem<size_t, double>>
  get_closest_index(Point &p, int N);
  std::vector<nanoflann::ResultItem<size_t, double>>
  get_closest_index(Point &p, double radius);

  std::map<const PIAnaHit *, std::vector<int>>
  get_hit_indices_map(const double radius);

protected:
  // std::vector<nanoflann::ResultItem<size_t, double>>
  // get_closest_index(PIPointCloud<double, PIAnaHit>::Point p,
  //                     double radius);
  PIPointCloud<double, PIAnaHit> cloud_;
  my_kd_tree_t *index;

  std::map<const PIAnaHit *, std::vector<int>> map_hit_indices_;

};
#endif
