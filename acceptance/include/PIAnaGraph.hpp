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
  std::map<int, IndicesType>
  connected_components(const double radius);

private:
  std::unique_ptr<PIAnaPointCloud> cloud_;
  std::unique_ptr<PIGraph> graph_;
};
#endif
