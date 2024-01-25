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
      Graph;
  typedef PIAnaPointCloud::IndexType IndexType;
  typedef PIAnaPointCloud::IndicesType IndicesType;

  PIAnaGraph();
  ~PIAnaGraph();
  void AddPoint(const PIAnaHit *);
  void clear();
  std::map<int, IndicesType>
  connected_components(const double radius);

private:
  std::unique_ptr<PIAnaPointCloud> cloud_;
  std::unique_ptr<Graph> graph_;
};
#endif
