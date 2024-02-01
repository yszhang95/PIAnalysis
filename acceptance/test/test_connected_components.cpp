#include <iostream>
#include <memory>

#include "PIAnaHit.hpp"
#include "PIAnaGraph.hpp"

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


int main() {
  const auto hits = create_vector();
  PIAnaGraph graph_xyz(3);
  PIAnaGraph graph_t(3);
  for (const auto &hit : hits) {
    graph_xyz.AddPoint(&hit);
    graph_t.AddPoint(&hit);
  }
  auto connected_components_xyz = graph_xyz.connected_components(0.15);
  auto connected_components_t = graph_t.connected_components(0.15);
  for (const auto &component : connected_components_xyz) {
    std::cout << "Component " << component.first << ": ( ";
    for (const auto idx : component.second) {
      std::cout << idx << " ";
    }
    std::cout << ").\n";
  }
  std::cout << "\n";

  for (const auto &component : connected_components_t) {
    std::cout << "Component " << component.first << ": ( ";
    for (const auto idx : component.second) {
      std::cout << idx << " ";
    }
    std::cout << ").\n";
  }
  std::cout << "\n";

  return 0;
}
