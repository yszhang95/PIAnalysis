#ifndef __PI_TreeAnalyzer__
#define __PI_TreeAnalyzer__
#include "PIEventAnalyzer.hpp"
#include "PIEventData.hpp"

#include "TROOT.h"
#include "Math/Vector3Dfwd.h"
#include "Math/Point3Dfwd.h"

#include <string>
#include <vector>

class TTree;
class TFile;
namespace PIAna
{
  class  PITreeAnalyzer: public PIEventAnalyzer
  {
  public:
    explicit PITreeAnalyzer(const std::string&);
    ~PITreeAnalyzer() override;
    void Begin() override;
    void DoAction(PIEventData&) override;
    void End() override;

  protected:
    void analyze(const PIEventData &evt) override;

  private:
    void clear();

    std::vector<std::vector<Float_t > > x_;
    std::vector<std::vector<Float_t > > y_;
    std::vector<std::vector<Float_t > > z_;
    std::vector<std::vector<Float_t > > t_;
    std::vector<std::vector<Float_t>> de_;

    ROOT::Math::XYZPoint *pivertex_;
    ROOT::Math::XYZPoint *estart_;
    ROOT::Math::Polar3DVector *edirection_;
    TTree *rec_tree_;
    int clusterid_[1000];
    int ncluster_;
  };
};

#endif
