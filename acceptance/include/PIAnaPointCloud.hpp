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

   The class save all data into a point cloud object. The hit object
   is described by `PIAnaHit` and referenced by pointers. Pointers
   which point to the same hit object is only stored in one slot.
   This may lead to a **different indexing** when inputing an vector
   of hits.
 */

class PIAnaPointCloud
{
public:
  typedef WCP::D3Vector<double> Point;
  typedef uint32_t IndexType;
  typedef std::vector<IndexType> IndicesType;

  typedef nanoflann::KDTreeSingleIndexAdaptor<
nanoflann::L2_Simple_Adaptor<double, PIPointCloud<double, PIAnaHit> > ,
  PIPointCloud<double, PIAnaHit>,
  3 /* dim */, IndexType
  > my_kd_tree_t;

  PIAnaPointCloud();
  ~PIAnaPointCloud();

  friend std::ostream &operator<<(std::ostream &os,
                                  const PIAnaPointCloud&);
  /**
     Add hit to point cloud when the hit is not recorded yet.
     @param hit It is a pointer to the hit object. Pointers to the same
                object is only added once.
   */
  void AddPoint(const PIAnaHit *hit);

  void clear();

  /**
     Build the k-d tree. It must be called before any call of
     1. get_closet_index()
     2. get_hit_indices_map()
   */
  void build_kdtree_index();

  size_t get_num_points() { return cloud_.pts.size(); }

  std::vector<nanoflann::ResultItem<IndexType, double>>
  get_closest_index(Point &p, int N);
  std::vector<nanoflann::ResultItem<IndexType, double>>
  get_closest_index(Point &p, double radius);

  /**
     Create a map between a hit, its index, its closest indices.
     The closest indices may contain the hit's index itself.
     The function must be called after build_kdtree_index().
     @return The map between a hit, its index, its closest indices.
   */
  std::map<const PIAnaHit *, std::pair<IndexType, IndicesType>>
  get_hit_indices_map(const double radius);

  /**
     A inspector method to access hit using the index for point
     cloud.
   */
  const PIAnaHit* get_hit(const IndexType idx) const;

protected:
  // std::vector<nanoflann::ResultItem<size_t, double>>
  // get_closest_index(PIPointCloud<double, PIAnaHit>::Point p,
  //                     double radius);
  PIPointCloud<double, PIAnaHit> cloud_;
  my_kd_tree_t *index;

  std::map<const PIAnaHit *, std::pair<IndexType, IndicesType> > map_hit_indices_;
};
#endif
