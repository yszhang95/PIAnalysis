#include <Rtypes.h>
#include <tuple>
#if defined(__CLING__)
R__LOAD_LIBRARY(../../install/lib/libPiAnaAcc.so)
R__LOAD_LIBRARY(../../install/lib/libPiRootDict.so)
#else
#include "PIAnaHit.hpp"
#endif
#include <iostream>

typedef std::tuple<double, double, double, double> mytuple;

PIAnaHit get_hit(const double t, const double x, const double y,
                 const double z)
{
  PIAnaHit hit;
  hit.dt(0);
  hit.post_t(t);
  hit.rec_x(x);
  hit.rec_y(y);
  hit.rec_z(z);
  return hit;
}

std::ostream& operator<<(std::ostream& os, const PIAnaHit& hit)
{
  os << "(" << hit.t() << ", " << hit.rec_x() << ", " << hit.rec_y() << ", "
     << hit.rec_z() << ")";
  return os;
}

void test_pi_hit(const mytuple h1, const mytuple h2)
{
  const auto hit1 = get_hit(std::get<0>(h1), std::get<1>(h1), std::get<2>(h1),
                            std::get<3>(h1));

  const auto hit2 = get_hit(std::get<0>(h2), std::get<1>(h2), std::get<2>(h2),
                            std::get<3>(h2));
  std::cout << "Compre htis " << hit1 << " < " << hit2 << " = " << (hit1 < hit2) << "\n";
}

void test_pi_hit()
{
  test_pi_hit({0.1, 1, 1, 1}, {0.2, 1, 0, 0});
  test_pi_hit({0, 1, 1, 1}, {0, 1, 0, 0});
  test_pi_hit({0, 1, 1, 1}, {0, 1, 1, 1});
  test_pi_hit({0, 1, 1, 1}, {0, 1, 1, 1});
  test_pi_hit({0, 1, 1, 1}, {0, 1, 1, 1});
  test_pi_hit({0, 0, 0, 0}, {0, 1, 0, 0});
  test_pi_hit({0, 1, 1, 0}, {0, 0, 0, 1});
}
