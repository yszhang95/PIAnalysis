// #include "TClonesArray.h"
#include "TTree.h"
#include "TFile.h"
#include "TError.h"
#include "Math/Vector3D.h"
#include "Math/Point3D.h"

#include "PITreeAnalyzer.hpp"
#include "PIAnaEvtBase.hpp"
#include "PIAnaHit.hpp"
#include "PIEventData.hpp"
#include "PIJobManager.hpp"
#include <Math/Point3Dfwd.h>
#include <Math/Vector3Dfwd.h>

PIAna::PITreeAnalyzer::PITreeAnalyzer(const std::string &name)
  : PIEventAnalyzer(name) {
  rec_tree_ = new TTree("rec_hits", "reconstructed hits");
  std::fill(std::begin(clusterid_), std::end(clusterid_), 0);

  pivertex_ = new ROOT::Math::XYZPoint();
  estart_ = new ROOT::Math::XYZPoint();
  edirection_ = new ROOT::Math::Polar3DVector();
}

PIAna::PITreeAnalyzer::~PITreeAnalyzer()
{
  delete pivertex_;
  pivertex_ = nullptr;
  delete estart_;
  estart_ = nullptr;
  delete edirection_;
  edirection_ = nullptr;
}

void PIAna::PITreeAnalyzer::Begin()
{
  PIEventAnalyzer::Begin();
  if (PIEventAction::mgr_->out_file()) {
    PIEventAnalyzer::outputfile_ = PIEventAction::mgr_->out_file();
  } else {
    Fatal("PIAna::PITreeAnalyzer::Begin()", "Output file is not proper initialized.");
  }
  if (PIEventAnalyzer::outputfile_) {
    rec_tree_->SetDirectory(PIEventAnalyzer::outputfile_);
    rec_tree_->Branch("ncluster", &ncluster_);
    rec_tree_->Branch("clusterid", clusterid_, "clusterid[ncluster]/I");
    rec_tree_->Branch("x", &this->x_);
    rec_tree_->Branch("y", &this->y_);
    rec_tree_->Branch("z", &this->z_);
    rec_tree_->Branch("t", &this->t_);
    rec_tree_->Branch("de", &this->de_);
    rec_tree_->Branch("pivertex", this->pivertex_);
    rec_tree_->Branch("estart", this->estart_);
    rec_tree_->Branch("edirection", this->edirection_);
  }
}

void PIAna::PITreeAnalyzer::DoAction(PIEventData &event)
{
  PIEventAnalyzer::DoAction(event);
}

void PIAna::PITreeAnalyzer::End() {
  PIEventAnalyzer::End();
}

void PIAna::PITreeAnalyzer::analyze(const PIEventData &event)
{
  clear();

  const std::map<const PIAnaHit *, std::vector<std::vector<const PIAnaHit *>>>
    &connected_hit_map =
    event.Get<const std::map<const PIAnaHit *,
                             std::vector<std::vector<const PIAnaHit *>>>>("Topo");
  const auto &pivertex_ref =
      event.Get<const ROOT::Math::XYZPoint>("Topo_pivertex");
  const auto &estart_ref = event.Get<const ROOT::Math::XYZPoint>("Topo_estart");

  const auto &edirection_ref = event.Get<const ROOT::Math::Polar3DVector>("Topo_edirection");

  pivertex_->SetXYZ(pivertex_ref.X(), pivertex_ref.Y(), pivertex_ref.Z());
  estart_->SetXYZ(estart_ref.X(), estart_ref.Y(), estart_ref.Z());
  std::vector<double> d3(3);
  edirection_ref.GetCoordinates(d3.begin(), d3.end());
  edirection_->SetCoordinates(d3.begin(), d3.end());

  int index = 0;
  for (auto it = connected_hit_map.begin(); it != connected_hit_map.end();
       ++it) {
    int code = (index++) * 100;
    for (const auto &hits : it->second) {
      code++;
      this->x_.push_back({});
      this->y_.push_back({});
      this->z_.push_back({});
      this->t_.push_back({});
      this->de_.push_back({});
      for (const auto hit : hits) {
        x_.back().push_back(hit->rec_x());
        y_.back().push_back(hit->rec_y());
        z_.back().push_back(hit->rec_z());
        t_.back().push_back(hit->t());
        de_.back().push_back(hit->edep());
      }
      clusterid_[this->ncluster_++] = code;
    }
  }

  rec_tree_->Fill();
}

void PIAna::PITreeAnalyzer::clear()
{
  std::fill(std::begin(clusterid_), std::end(clusterid_), 0);
  x_.clear();
  y_.clear();
  z_.clear();
  t_.clear();
  de_.clear();
  ncluster_ = 0;

  pivertex_->SetXYZ(0, 0, 0);
  estart_->SetXYZ(0, 0, 0);
  edirection_->SetCoordinates(0, 0, 0);
}
