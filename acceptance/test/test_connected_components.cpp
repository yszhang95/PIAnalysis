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
  PIAnaGraph graph_t(1);
  for (size_t idx = 0; idx != hits.size(); ++idx) {
    const auto& hit = hits.at(idx);
    graph_xyz.AddPoint(&hit);
    graph_t.AddPoint(&hit);
    std::cout << "Added point (idx, t, x, y, z) = (" << idx << ", " << hit.t()
              << ", "
              << hit.rec_x() << ", " << hit.rec_y() << ", " << hit.rec_z() << ").\n";
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

  std::cout << "Checking connected point to reference point\n";
  std::cout << "Reference XYZ=(0.005, 0.005, 0.6), radius=0.15\n";
  PIAnaPointCloud::Point  xyz0 {0.005, 0.005, 0.6};
  auto connected_to_ref_xyz = graph_xyz.connected_components(xyz0, 0.15);
  std::cout << "Connected components are ( ";
  for (const auto idx : connected_to_ref_xyz) {
    std::cout << idx << " ";
  }
  std::cout << ").\n";
  std::cout << "Reference T=5.002, radius = 4.8\n";
  PIAnaPointCloud::Point t0{5.002, 0, 0};
  auto connected_to_ref_t = graph_t.connected_components(t0, 4.9);
  std::cout << "Connected components are ( ";
  for (const auto idx : connected_to_ref_t) {
    std::cout << idx << " ";
  }
  std::cout << ").\n";

  return 0;
}
