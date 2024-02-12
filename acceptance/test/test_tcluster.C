#include <iostream>
#include <memory>

#if defined(__CLING__)
#include <Rtypes.h>
R__LOAD_LIBRARY(../../install/lib/libPiAnaAcc.so)
R__LOAD_LIBRARY(../../install/lib/libPiRootDict.so)
#elif defined(__ROOT_CLING__)
#else
#include "PIAnaHit.hpp"
#include "PIAnaGraph.hpp"
#include "PIAnaCluster.hpp"
#endif

std::ostream& operator<<(std::ostream& os, const PIAnaHit& hit)
{
  os << "(" << hit.t() << ", " << hit.rec_x() << ", " << hit.rec_y() << ", "
     << hit.rec_z() << ")";
  return os;
}

std::vector<PIAnaHit> create_vector() {
  std::vector<PIAnaHit> results;

  std::vector<float> xtemp, ytemp, ztemp, dedxtemp;
  for (int i = 0; i < 30; ++i) {
    auto hit = PIAnaHit();
    hit.post_t(0.001 + 0.001*i);
    hit.dt(0.001);
    hit.rec_x(i * 0.001);
    hit.rec_y(i * 0.001);
    hit.rec_z(i * 0.12);
    results.push_back(hit);
  }
  for (int i=0; i<4; ++i)
  {
    auto hit = PIAnaHit();
    hit.post_t(5 + 0.001*i);
    hit.dt(0.001);
    hit.rec_x(2 + i * 0.001);
    hit.rec_y(2 + i * 0.001);
    hit.rec_z(1 + i * 0.12);
    results.push_back(hit);
  }

  return results;
}


void test_tcluster()
{
  const auto hits = create_vector();
  std::vector<const PIAnaHit *> hitptrs;
  for (const auto& hit : hits) {
    hitptrs.push_back(&hit);
  }
  PITCluster tcluster;
  tcluster.t0(0.002);
  tcluster.radius(1);
  const auto clusters = tcluster.cluster_hits(hitptrs.begin(), hitptrs.end());

  for (const auto &cluster : clusters) {
    std::cout << "Reference point: " << cluster.first->t() << ", ";
    std::cout << "connected hits: ";
    for (const auto &hit : cluster.second) {
      std::cout << *hit << " ";
    }
    std::cout << "\n\n";
  }
}

int main()
{
  test_tcluster();
  return 0;
}
