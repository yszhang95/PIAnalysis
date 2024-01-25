#include <iostream>
#include <memory>

#include "PIAnaHit.hpp"
#include "PIAnaGraph.hpp"

std::vector<PIAnaHit> create_vector() {
  std::vector<PIAnaHit> results;

  std::vector<float> xtemp, ytemp, ztemp, dedxtemp;
  for (int i = 0; i < 30; ++i) {
    auto hit = PIAnaHit();
    hit.rec_x(i * 0.001);
    hit.rec_y(i * 0.001);
    hit.rec_z(i * 0.12);
    results.push_back(hit);
  }
  for (int i=0; i<4; ++i)
  {
    auto hit = PIAnaHit();
    hit.rec_x(2 + i * 0.001);
    hit.rec_y(2 + i * 0.001);
    hit.rec_z(1 + i * 0.12);
    results.push_back(hit);
  }

  return results;
}


int main() {
  const auto hits = create_vector();
  PIAnaGraph graph;
  for (const auto &hit : hits) {
    graph.AddPoint(&hit);
  }
  auto connected_components = graph.connected_components(0.15);
  for (const auto &component : connected_components) {
    std::cout << "Component " << component.first << ": ( ";
    for (const auto idx : component.second) {
      std::cout << idx << " ";
    }
    std::cout << ").\n";
  }
  std::cout << "\n";
  return 0;
}
