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
      nanoflann::L2_Simple_Adaptor<double, PIPointCloud<double, PIAnaHit>>,
      PIPointCloud<double, PIAnaHit>, 3 /* dim */, IndexType>
      my_kd_tree_3d_t;

  typedef nanoflann::KDTreeSingleIndexAdaptor<
    nanoflann::L2_Simple_Adaptor<double, PIPointCloud<double, PIAnaHit> > ,
    PIPointCloud<double, PIAnaHit>,
    1 /* dim */, IndexType
    > my_kd_tree_1d_t;

  virtual ~PIAnaPointCloud();

  friend std::ostream &operator<<(std::ostream &os,
                                  const PIAnaPointCloud&);
  /**
     Add hit to point cloud when the hit is not recorded yet.
     @param hit It is a pointer to the hit object. Pointers to the same
                object is only added once.
   */
  virtual void AddPoint(const PIAnaHit *hit) = 0;

  virtual void clear();

  /**
     Build the k-d tree. It must be called before any call of
     1. get_closet_index()
     2. get_hit_indices_map()
   */
  virtual void build_kdtree_index() = 0;

  size_t get_num_points() { return cloud_.pts.size(); }

  virtual std::vector<nanoflann::ResultItem<IndexType, double>>
  get_closest_index(const Point &p, int N) = 0;
  virtual std::vector<nanoflann::ResultItem<IndexType, double>>
  get_closest_index(const Point &p, double radius) = 0;

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
  PIAnaPointCloud();
  virtual Point get_point(const PIAnaHit *) = 0;
  void AddPoint(const Point &p, const PIAnaHit *hit);

  PIPointCloud<double, PIAnaHit> cloud_;
  std::map<const PIAnaHit *, std::pair<IndexType, IndicesType>>
      map_hit_indices_;

private:
  // copy constructor
  PIAnaPointCloud(PIAnaPointCloud const &other) = delete;
  // copy assignment
  PIAnaPointCloud &operator=(PIAnaPointCloud const &other) = delete;
  // move constructor
  PIAnaPointCloud(PIAnaPointCloud&& other) = delete;
};

class PIAnaPointCloud3D : public PIAnaPointCloud
{
public:
  ~PIAnaPointCloud3D();

  void build_kdtree_index() final;
  std::vector<nanoflann::ResultItem<IndexType, double>>
  get_closest_index(const Point &p, int N) final;
  std::vector<nanoflann::ResultItem<IndexType, double>>
  get_closest_index(const Point &p, double radius) final;

  void clear() final;

  void AddPoint(const PIAnaHit *hit) override = 0;

protected:
  PIAnaPointCloud3D();
  Point get_point(const PIAnaHit*) override = 0;
  my_kd_tree_3d_t *index;

private:
  // copy constructor
  PIAnaPointCloud3D(PIAnaPointCloud3D const &other) = delete;
  // copy assignment
  PIAnaPointCloud3D &operator=(PIAnaPointCloud3D const &other) = delete;
  // move constructor
  PIAnaPointCloud3D(PIAnaPointCloud3D&& other) = delete;
};

class PIAnaPointCloudXYZ : public PIAnaPointCloud3D
{
public:
  explicit PIAnaPointCloudXYZ();
  ~PIAnaPointCloudXYZ();
  void AddPoint(const PIAnaHit *hit) final;

protected:
  Point get_point(const PIAnaHit*) final;

private:
  // copy constructor
  PIAnaPointCloudXYZ(PIAnaPointCloudXYZ const &other) = delete;
  // copy assignment
  PIAnaPointCloudXYZ &operator=(PIAnaPointCloudXYZ const &other) = delete;
  // move constructor
  PIAnaPointCloudXYZ(PIAnaPointCloudXYZ&& other) = delete;
};

class PIAnaPointCloud1D : public PIAnaPointCloud
{
public:
  ~PIAnaPointCloud1D();

  void build_kdtree_index() final;
  std::vector<nanoflann::ResultItem<IndexType, double>>
  get_closest_index(const Point &p, int N) final;
  std::vector<nanoflann::ResultItem<IndexType, double>>
  get_closest_index(const Point &p, double radius) final;

  void clear() final;

  void AddPoint(const PIAnaHit *hit) override = 0;

protected:
  PIAnaPointCloud1D();
  Point get_point(const PIAnaHit*) override = 0;
  my_kd_tree_1d_t *index;

private:
  // copy constructor
  PIAnaPointCloud1D(PIAnaPointCloud1D const &other) = delete;
  // copy assignment
  PIAnaPointCloud1D &operator=(PIAnaPointCloud1D const &other) = delete;
  // move constructor
  PIAnaPointCloud1D(PIAnaPointCloud1D&& other) = delete;
};

class PIAnaPointCloudT : public PIAnaPointCloud1D
{
public:
  PIAnaPointCloudT();
  ~PIAnaPointCloudT();

  void AddPoint(const PIAnaHit *hit) override;

protected:
  Point get_point(const PIAnaHit *) override;

private:
  // copy constructor
  PIAnaPointCloudT(PIAnaPointCloudT const &other) = delete;
  // copy assignment
  PIAnaPointCloudT &operator=(PIAnaPointCloudT const &other) = delete;
  // move constructor
  PIAnaPointCloudT(PIAnaPointCloudT&& other) = delete;
};
#endif
