#ifndef __PI_AnaGraph__
#define __PI_AnaGraph__

#include <map>
#include <memory>
#include <vector>

#include <boost/graph/adjacency_list.hpp>

#include "PIAnaPointCloud.hpp"
class PIAnaHit;
class PIAnaGraph
{
public:
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>
      PIGraph;
  typedef PIAnaPointCloud::IndexType IndexType;
  typedef PIAnaPointCloud::IndicesType IndicesType;

  /**
   * A helper class to create the graph for connected points.
   * Points are connected by timing, (dim=1) or position (dim=3).
   * An **enumeration** class may be used in the future.
   * @param dim it is the dimension of the point.
   */
  PIAnaGraph(const unsigned int dim);
  ~PIAnaGraph();
  void AddPoint(const PIAnaHit *);
  void clear();
  IndicesType::size_type get_num_points() { return cloud_->get_num_points(); }

  const PIAnaHit* get_hit(IndexType idx) const
  { return cloud_->get_hit(idx); }

  /**
   * @param radius The search radius. Hits are connected
   * if their distance is within the raidus.
   * @return The map between i-th connected component and the indices
   * for its content in the point cloud.
   */
  std::map<int, IndicesType> connected_components(const double radius);

  /**
   * Find out hits connected to the reference point given the search radius.
   * @param point Reference point for search within radius.
   * @param radius Search radius.
   * @return The collection of indices for connected hits to reference
   * point in the point cloud.
   */
  IndicesType
  connected_components(const PIAnaPointCloud::Point& point, const double radius);

    /**
   * Find out N hits connected to the reference point.
   * @param point Reference point for search within radius.
   * @param N N neartest points.
   * @return The collection of indices for connected hits to reference
   * point in the point cloud.
   */
  IndicesType
  connected_components(const PIAnaPointCloud::Point& point, const int N);
private:
  std::unique_ptr<PIAnaPointCloud> cloud_;
  std::unique_ptr<PIGraph> graph_;
};
#endif
