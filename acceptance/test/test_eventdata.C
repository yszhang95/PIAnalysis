#include <Math/Point3Dfwd.h>
#include <Math/Point3D.h>
#include <Rtypes.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TRandom3.h>
#ifdef __CLING__
R__LOAD_LIBRARY(../../../PIAnalysis_install/lib/libPiAnaAcc.so)
#else
#include "PIEventData.hpp"
#endif

struct point {
  float x;
  float y;
  float z;
  point(float x, float y, float z) : x(x), y(y), z(z) {}
  void set_point(float x, float y, float z) {
    this->x = x;
    this->y = y;
    this->z = z;
  }
};

void print(PIAna::PIEventData& event) {
  const auto &ptrs = event.Get<std::vector<const point *>>("ptrs");
  for (const auto ptr : ptrs) {
    std::cout << ptr->x << ", " << ptr->y << ", " << ptr->z << "\n";
  }
}

void draw(TH1* h, const char*);

void test_eventdata()
{
  PIAna::PIEventData event;
  ROOT::Math::XYZPoint p1(1, 2, 3);
  event.Put("p1", p1);
  const auto &p1ref = event.Get<ROOT::Math::XYZPoint>("p1");
  std::cout << p1.x() << " == " << p1ref.x() << "\n";

  TH2F *h1 = new TH2F("h1", "h1", 60, -3, 3, 60, -3, 3);
  TH2F h2("h2", "h2", 60, -3, 3, 60, -3, 3);
  TH1F *h3 = new TH1F("h3", "h3", 60, -3, 3);
  TH1F h4("h4", "h4", 60, -3, 3);

  for (int i = 0; i < 5000; ++i) {
    h1->Fill(gRandom->Gaus(0, 1), gRandom->Gaus(0, 1));
    h2.Fill(gRandom->Gaus(0, 1), gRandom->Gaus(0, 1));
    h3->Fill(gRandom->Gaus(0, 1));
    h4.Fill(gRandom->Gaus(0, 1));
  }

  event.Put("h1", h1);
  event.Put("h2", h2);
  event.Put("h3", h3);
  event.Put("h4", h4);

  std::vector<point> ps;
  for (int i = 0; i < 10; ++i) {
    ps.emplace_back(i, i, i);
  }
  event.Put("points", ps);

  const auto &points = event.Get<std::vector<point>>("points");

  std::vector<const point*> ptrs;
  for (const auto &p : points) {
    const point *ptr = &p;
    ptrs.push_back(ptr);
  }
  event.Put<std::vector<const point *>>("ptrs", ptrs);

  print(event);

  draw(event.Get<TH2F *>("h1"), "h1.png");
  // draw(&event.Get<TH2F>("h2"), "h2.png");
  draw(event.Get<TH1F *>("h3"), "h3.png");
  // draw(&event.Get<TH1F>("h4"), "h4.png");

  delete h1;
  delete h3;
}

void draw(TH1* h, const char* s)
{
  TCanvas c;
  if (dynamic_cast<TH1F *>(h)) {
    h->Draw();
  } else if (dynamic_cast<TH2F *>(h)) {
    h->Draw("COLZ");
  }
  c.Print(s);
}
