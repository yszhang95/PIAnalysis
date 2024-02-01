#include <iostream>

#include "PIAnaHit.hpp"
#include "PIAnaPointCloud.hpp"
#include "nanoflann.hpp"

std::vector<PIAnaHit> create_vector() {
  std::vector<PIAnaHit> results;

  std::vector<float> xtemp, ytemp, ztemp, dedxtemp;
  for (int i = 0; i < 30; ++i) {
    auto hit = PIAnaHit();
    hit.dt(0.001);
    hit.post_t((i+1)*0.001);
    hit.x(i * 0.001);
    hit.y(i * 0.001);
    hit.z(i * 0.12);
    hit.rec_x(i * 0.001);
    hit.rec_y(i * 0.001);
    hit.rec_z(i * 0.12);
    results.push_back(hit);
  }

  return results;
}

int main()
{
  auto hits = create_vector();
  PIAnaPointCloudXYZ cloud;
  PIAnaPointCloudT cloud2;
  std::cout << "PointCloudXYZ\n" << cloud;
  std::cout << "PointCloudT\n" << cloud2;
  // test AddPoint
  for (const auto &hit : hits) {
    cloud.AddPoint(&hit);
    cloud2.AddPoint(&hit);
    // std::cout << cloud;
  }
  std::cout << "PointCloudXYZ\n" << cloud;
  std::cout << "PointCloudT\n" << cloud2;

  // build k-d tree
  cloud.build_kdtree_index();
  cloud2.build_kdtree_index();
  // study the nearest points in the third point
  // helper for printing out
  auto print_search_by_radius =
      [](const PIAnaHit &hit, double raidus,
         const std::vector<nanoflann::ResultItem<PIAnaPointCloud::IndexType, double>> &pts) {
        std::cout << "The closest points within raidus " << raidus << " to ("
                  << hit.rec_x() << ", " << hit.rec_y() << ", " << hit.rec_z()
                  << ") have (index, distance): ";
        for (const auto &p : pts) {
          std::cout << "(" << p.first << ", " << p.second << ") ";
        }
        std::cout << "\n";
      };
  auto print_search_by_N =
      [](const PIAnaHit &hit, int N,
         const std::vector<nanoflann::ResultItem<PIAnaPointCloud::IndexType, double>> &pts) {
        std::cout << "The "<< N << " closest points"
                << " to (" << hit.rec_x() << ", "
                << hit.rec_y() << ", "
                << hit.rec_z()
                << ") have (index, distance): ";
      for (const auto &p : pts) {
        std::cout << "(" << p.first << ", " << p.second << ") ";
      }
      std::cout << "\n";
    };

  // test point
  PIAnaPointCloud::Point p(hits.at(2).rec_x(), hits.at(2).rec_y(),
                           hits.at(2).rec_z());
  // test get_closest_index(point, radius);
  auto ps = cloud.get_closest_index(p, 0.15);
  print_search_by_radius(hits.at(2), 0.15, ps);
  // test get_closest_index(point, N);
  auto ps2 = cloud.get_closest_index(p, 3);
  print_search_by_N(hits.at(2), 3, ps2);

  // test get_hit_indices_map
  cloud.get_hit_indices_map(0.15);
  std::cout << cloud << "\n";
  cloud2.get_hit_indices_map(0.00199);
  std::cout << cloud2 << "\n";

  // test get_hit
  const PIAnaHit *hit = cloud.get_hit(20);
  std::cout << "The 20th hit is located at (" << hit->rec_x() << ", "
            << hit->rec_y() << ", " << hit->rec_z() << ").\n";
  return 0;
}
