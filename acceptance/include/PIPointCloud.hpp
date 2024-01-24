#ifndef __PI_PointCloud__
#define __PI_PointCloud__
#include <cstddef>
#include <vector>
template<typename T, typename Hit>
  struct PIPointCloud
  {
    struct Point
    {
      T x, y, z;
      const Hit* hit;
      int index_x, index_y, index_z;
      int index;
    };
    std::vector<Point> pts;
    inline size_t kdtree_get_point_count()
      const { return pts.size(); }
    inline Hit* get_hit(const size_t idx) const
    { return pts[idx].hit; }

    inline T kdtree_get_pt(const size_t idx, int dim) const
    {
      if (dim == 0) return pts[idx].x;
      if (dim == 1) return pts[idx].y;
      else return pts[idx].z;
    }

    // Optional bounding-box computation: return false to default to a standard bbox computation loop.
    //   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
    //   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
    template <class BBOX>
    bool kdtree_get_bbox(BBOX& /* bb */) const { return false; }
};

#endif
