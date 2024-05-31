#include "TClonesArray.h"
#include "TError.h"
#include "TMath.h"
#include "Math/Point3Dfwd.h"
#include "Math/Vector3Dfwd.h"
#include "Math/Vector3D.h"
#include "Math/PositionVector3D.h"

#include "PIAnaCluster.hpp"
#include "PITopoProducer.hpp"
#include "PITkFinder.hpp"
#include "PIAnaEvtBase.hpp"
#include "PIAnaHit.hpp"
#include "PIEventData.hpp"
#include <Math/GenVector/VectorUtil.h>

PIAna::PITopoProducer::PITopoProducer(const std::string &name)
  : PIEventProducer(name), rec_hits_name_("RecHits"),
    tcluster_dt_(5), dt_(1), dl_(0.2), min_tcluster_(1)
  {
    dl_ = std::sqrt(0.2*0.2*2 + 0.12*0.12)*3; // some random guess of 3 pixels
    tcluster_ = std::make_unique<PITCluster>();
    xyzcluster_ = std::make_unique<PIXYZCluster>();
  }

PIAna::PITopoProducer::~PITopoProducer() {}

void PIAna::PITopoProducer::Begin()
{
  PIEventProducer::Begin();

  tcluster_->radius(dt_);
  xyzcluster_->radius(dl_);

  if (PIEventAction::verbose_) {
    report();
  }
}

void PIAna::PITopoProducer::report() {
  std::vector<std::string> msgs;

  msgs.emplace_back(::Form("PITopoProducer: %s", PIEventProducer::GetName().c_str()));
  msgs.emplace_back(::Form("Input hit collection: %s", rec_hits_name_.c_str()));

  msgs.emplace_back(::Form("Minimum timing separation between t-clusters: %f [ns]", tcluster_dt_));

  msgs.emplace_back(::Form("Search radius for t-cluster: %f [ns]", dt_));

  msgs.emplace_back(::Form("Search radius for xyz-cluster: %f [mm]", dl_));

  msgs.emplace_back(::Form("Minimum number of t-cluster: %u", min_tcluster_));

  for (const auto &msg : msgs) {
    Info("PIAna::PITopoProducer", msg.c_str());
  }
}

void PIAna::PITopoProducer::DoAction(PIEventData &event)
{
  PIEventProducer::DoAction(event);
}

void PIAna::PITopoProducer::End() { PIEventProducer::End(); }



void PIAna::PITopoProducer::produce(PIEventData &event)
{
  const std::vector<PIAnaHit> &rec_hits =
      event.Get<std::vector<PIAnaHit>>(rec_hits_name_);

  std::vector<const PIAnaHit *> hits;
  for (const auto &hit : rec_hits) {
    hits.emplace_back(&hit);
  }

  std::vector<const PIAnaHit*> hit0s;
  for (const auto &hit : rec_hits) {
    if (hit.layer() == 0) {
      hit0s.push_back(&hit);
    }
  }
  if (hit0s.empty()) {
    Warning("PIAna::PITopoProducer::produce",
            "Not found any hits on the first layer.");
    fill_dummy(event);
    return;
  }
  std::sort(
            std::begin(hit0s), std::end(hit0s),
            [](const PIAnaHit *h1, const PIAnaHit *h2) { return h1->t() < h2->t(); });

  tcluster_->t0(hit0s.front()->t());

  std::vector<const PIAnaHit *> hitptrs;
  hitptrs.reserve(rec_hits.size());
  // must be auto& hit
  for (const auto &hit : rec_hits) {
    hitptrs.push_back(&hit);
  }
  const auto tclusters =
      tcluster_->cluster_hits(hitptrs.begin(), hitptrs.end());

  std::vector<double> t0s;
  for (const auto &cluster : tclusters) {
    t0s.push_back(cluster.first->t());
  }
  std::sort(std::begin(t0s), std::end(t0s));
  // first iteration
  double t0_temp = t0s.front();
  int nt0s = 0;
  for (size_t i = 0; i != t0s.size(); ++i) {
    if (t0s.at(i) - t0_temp > tcluster_dt_) {
      t0_temp = t0s.at(i);
      nt0s++;
    }
  }
  if ((nt0s + 1) < min_tcluster_) {
    Warning("PIAna::PITopoProducer::produce", "Merged topology by time.");
    fill_dummy(event);
    return;
  }

  for (const auto &cluster : tclusters) {
    const double dt = cluster.first->t() - tcluster_->t0();
    if (std::abs(dt) < tcluster_->radius())
      continue;
  }

  // find out pi vertex from earliest cluster
  const PIAnaHit* hitbyz = nullptr;
  {
    const PIAnaHit *ptr = nullptr;
    double t0 = 1E25;
    for (const auto &cluster : tclusters) {
      if (cluster.first->t() < t0) {
        ptr = cluster.first;
        t0 = cluster.first->t();
      }
    }
    std::vector<const PIAnaHit *> hits_copy = tclusters.at(ptr);
    if (hits_copy.empty()) {
      Warning("PIAna::PITopoProducer::produce", "Why the collection is empty for prompt timing?");
      fill_dummy(event);
      return;
    }
    // by z
    std::sort(hits_copy.begin(), hits_copy.end(),
              [](const PIAnaHit *h1, const PIAnaHit *h2) {
                return h1->rec_z() > h2->rec_z();
              });
    hitbyz = hits_copy.front();
  }

  // xyz cluster
  std::map<const PIAnaHit *, std::vector<std::vector<const PIAnaHit *>>>
      txyz_connected_hits;
  for (const auto &tcluster : tclusters) {
    txyz_connected_hits.insert({tcluster.first, {}});
    auto clusters = xyzcluster_->cluster_hits(tcluster.second.begin(),
                                              tcluster.second.end());
    for (const auto& hits : clusters) {
      txyz_connected_hits.at(tcluster.first).push_back(hits.second);
    }
  }
  // find decay point
  // refine later
  // the first one is pion, may be wrong

  // hard code // cannot handle all cases
  // use the last collection in timing
  // I asumme there is only one collection for each timing
  const PIAnaHit *lastptr = nullptr;
  // for (const auto &hit : txyz_connected_hits)
  for (const auto& hit : tclusters)
    {
      if (lastptr) {
        if (lastptr->t() < hit.first->t()) {
          lastptr = hit.first;
        }
      } else {
        lastptr = hit.first;
      }
  }

  // if (txyz_connected_hits.at(lastptr).size() != 1) {
  //   char msg[200];
  //   sprintf(msg,
  //           "%zu connected xyz components. "
  //           "Run:Event:EventID=%d:%d:%d", txyz_connected_hits.at(lastptr).size(),
  //           info_->GetRun(), info_->GetEvent(), info_->GetEventID());
  //   Error("PIAnalyzer::analyze_atar_hits", msg);
  //   for (const auto &cluster : txyz_connected_hits.at(lastptr)) {
  //     std::cout << "Cluster t0 " << lastptr->t() << "\n";
  //     for (const auto &hit : cluster) {
  //       std::cout << hit->t() << "\t" << hit->xstrip() << "\t" << hit->ystrip() << "\t" << hit->layer() << "\t" << hit->pdgid()<<"\n";
  //     }
  //   }
  // }

  PIAnaPointCloud::Point pivertex{hitbyz->rec_x(), hitbyz->rec_y(), hitbyz->rec_z()};

  // const auto posi_dir =
  //     positron_direction(txyz_connected_hits.at(lastptr).front(), pivertex);
  std::vector<ROOT::Math::XYZPoint> ehits;
  ROOT::Math::XYZPoint estart;
  const auto posi_dir = positron_direction(tclusters.at(lastptr), pivertex, estart, ehits);
  if (posi_dir.first) {
    if (hitbyz->xstrip() < 90 && hitbyz->xstrip() >= 10 &&
        hitbyz->ystrip() < 90 && hitbyz->ystrip() >= 10) {
    }
  }

  ROOT::Math::XYZPoint pivtx{hitbyz->rec_x(), hitbyz->rec_y(), hitbyz->rec_z()};
  event.Put<ROOT::Math::XYZPoint>(::Form("%s_pivertex", PIEventProducer::GetName().c_str())
                                  , pivtx);
  event.Put<ROOT::Math::XYZPoint>(
      ::Form("%s_estart", PIEventProducer::GetName().c_str()), estart);
  event.Put<ROOT::Math::Polar3DVector>(
      ::Form("%s_edirection", PIEventProducer::GetName().c_str()),
      posi_dir.second);
  event.Put<bool>(::Form("%s_flag", PIEventProducer::GetName().c_str()), posi_dir.first);

  event.Put<
      std::map<const PIAnaHit *, std::vector<std::vector<const PIAnaHit *>>>>(
      PIEventProducer::GetName(), txyz_connected_hits);

  event.Put<std::vector<ROOT::Math::XYZPoint>>(
      ::Form("%s_ehits", PIEventProducer::GetName().c_str()),
      ehits);
}

void PIAna::PITopoProducer::fill_dummy(PIAna::PIEventData &event) {
  // hard code
  event.Put<ROOT::Math::XYZPoint>(::Form("%s_pivertex", PIEventProducer::GetName().c_str())
                                  , {-1E9, -1E9, -1E9});
  event.Put<ROOT::Math::XYZPoint>(
      ::Form("%s_estart", PIEventProducer::GetName().c_str()), {});
  event.Put<ROOT::Math::Polar3DVector>(::Form("%s_edirection", PIEventProducer::GetName().c_str())
                                  , {});
  event.Put<
      std::map<const PIAnaHit *, std::vector<std::vector<const PIAnaHit *>>>>(
      PIEventProducer::GetName(), {});
  event.Put<bool>(::Form("%s_flag", PIEventProducer::GetName().c_str()), false);

  event.Put<std::vector<ROOT::Math::XYZPoint>>(
      ::Form("%s_ehits", PIEventProducer::GetName().c_str()),
      {});
}

std::vector<std::pair<const PIAnaHit *, const PIAnaHit *>>
PIAna::PITopoProducer::decay_point(const std::vector<const PIAnaHit *> &,
                                   const std::vector<const PIAnaHit *> &)
                                   {
                                     return {};
}

std::pair<bool, ROOT::Math::Polar3DVector>
PIAna::PITopoProducer::positron_direction(
    const std::vector<const PIAnaHit *> hits,
    const PIAnaPointCloud::Point pivertex, ROOT::Math::XYZPoint &estart,
    std::vector<ROOT::Math::XYZPoint> &ehits)
    {

  if (hits.size() < 5) {
    Warning("PIAna::PITopoProducer::position_direction", "The size of input positron hit collection is less than 5.");
      return {false, {}};
  }

  auto xyzgraph = PIAnaGraph(3);
  for (const auto& hit : hits) {
    xyzgraph.AddPoint(hit);
  }
  const auto indices = xyzgraph.connected_components(pivertex, 5);
  if (indices.size() < 5) {
    Warning("PIAna::PITopoProducer::position_direction", "The size of serach by graph is less than 5.");
      return {false, {}};
  }

  auto pca = PIAna::PITkPCA(3);
  for (unsigned int i = 0; i != 5; ++i) {
    const auto hit = xyzgraph.get_hit(indices.at(i));
    double v[3] = {hit->rec_x(), hit->rec_y(), hit->rec_z()};
    pca.add_data(v);
  }
  pca.fit();

  // reference point
  const auto piref = ROOT::Math::XYZPoint(pivertex.x, pivertex.y, pivertex.z);
  const auto iter1_dire = pca.get_direction();

  // the second iteration
  auto extremes = [&piref, &iter1_dire](const PIAnaHit *h1, const PIAnaHit *h2) -> bool {
    const auto p1 = ROOT::Math::XYZPoint(h1->rec_x(), h1->rec_y(), h1->rec_z());
    const auto p2 = ROOT::Math::XYZPoint(h2->rec_x(), h2->rec_y(), h2->rec_z());
    const auto d1 = p1 - piref;
    const auto d2 = p1 - piref;
    return d1.Dot(iter1_dire) < d1.Dot(iter1_dire);
  };
  const auto p_extremes = std::minmax_element(hits.begin(), hits.end(), extremes);
  const PIAnaHit *const ptr1 = *p_extremes.first;
  const PIAnaHit* const ptr2 = *p_extremes.second;
  const auto p1 =
      ROOT::Math::XYZPoint(ptr1->rec_x(), ptr1->rec_y(), ptr1->rec_z());
  const auto p2 =
      ROOT::Math::XYZPoint(ptr2->rec_x(), ptr2->rec_y(), ptr2->rec_z());
  const auto eposition = (p1 - piref).Mag2() < (p2 - piref).Mag2() ? p1 : p2;

  estart.SetX(eposition.X());
  estart.SetY(eposition.Y());
  estart.SetZ(eposition.Z());

  const PIAnaPointCloud::Point evertex{eposition.X(), eposition.Y(),
                                       eposition.Z()};
  pca.clear_data();

  const auto indices2 = xyzgraph.connected_components(evertex, 5);
  for (unsigned int i=0; i != indices2.size(); ++i) {
    const auto hit = xyzgraph.get_hit(indices2.at(i));
    double v[3] = {hit->rec_x(), hit->rec_y(), hit->rec_z()};
    pca.add_data(v);
  }
  pca.fit();

  for (unsigned int i = 0; i != indices2.size(); ++i) {
    const auto hit = xyzgraph.get_hit(indices2.at(i));
    ehits.emplace_back(hit->rec_x(), hit->rec_y(), hit->rec_z());
  }

  const auto iter2_dire = pca.get_direction();
  // justify new direction
  ROOT::Math::XYZVector n =
      ROOT::Math::XYZPoint{xyzgraph.get_hit(indices2.back())->rec_x(),
                           xyzgraph.get_hit(indices2.back())->rec_y(),
                           xyzgraph.get_hit(indices2.back())->rec_z()} -
      estart;
  ROOT::Math::Polar3DVector ret;
  if (n.Dot(iter2_dire) < 0) {
    const double theta = TMath::Pi() - iter2_dire.Theta();
    const double phi = ROOT::Math::VectorUtil::Phi_mpi_pi(iter2_dire.Phi() + TMath::Pi());
    ret.SetCoordinates(1,
                       theta, phi);
  } else {
    ret.SetCoordinates(1, iter2_dire.Theta(), iter2_dire.Phi());
  }
  return {true, ret};
}
