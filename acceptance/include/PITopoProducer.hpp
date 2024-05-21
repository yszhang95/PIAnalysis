#ifndef __PI_TopoProducer__
#define __PI_TopoProducer__
#include <Math/Point3Dfwd.h>
#include <memory>
#include <vector>

#include "PIEventProducer.hpp"
#include "PIAnaPointCloud.hpp"
#include "Math/Vector3Dfwd.h"

class PIAnaHit;
class PITCluster;
class PIXYZCluster;

namespace PIAna
{
  class PITopoProducer : public PIEventProducer
  {
  public:
    PITopoProducer(const std::string &);
    ~PITopoProducer();

    void Begin() override;
    void DoAction(PIEventData &evt) override;
    void End() override;

    std::vector<std::pair<const PIAnaHit *, const PIAnaHit *>>
    decay_point(const std::vector<const PIAnaHit *> &,
                const std::vector<const PIAnaHit *> &);

    std::pair<bool, ROOT::Math::Polar3DVector>
    positron_direction(const std::vector<const PIAnaHit *> hits,
                       const PIAnaPointCloud::Point pivertex,
                       ROOT::Math::XYZPoint &,
                       std::vector<ROOT::Math::XYZPoint> &);


    /**
     * Time separations between t-clusters.
     */
    void tcluster_dt(const double dt) { tcluster_dt_ = dt; }
    double tcluster_dt() const { return tcluster_dt_; }

    /**
     * Minimum number of t-clusters per event.
     * The event will be skipped if number of t-clusters < minimum.
     */
    void min_tcluster(const unsigned int n) { min_tcluster_ = n; }
    unsigned int min_tcluster() const { return min_tcluster_; }

    /**
     * Key name for input hits.
     */
    void rec_hit_name(const std::string n) { rec_hits_name_ = n; }
    const std::string rec_hit_name() const { return rec_hits_name_; }

    /**
     * Search radius in ns for t-cluster.
     */
    void dt(const double dt) { dt_ = dt; }
    double dt() const { return dt_; }
    /**
     * Search radius in mm for xyz-cluster
     */
    void dl(const double dl) { dl_ = dl; }
    double dl() const { return dl_; }

  protected:
    void produce(PIEventData &) override;
    void fill_dummy(PIEventData &) override;
    void report() override;

  private:
    std::unique_ptr<PITCluster> tcluster_;
    std::unique_ptr<PIXYZCluster> xyzcluster_;

    std::string rec_hits_name_;
    double tcluster_dt_; // ns
    double dt_; // ns
    double dl_; // mm

    unsigned int min_tcluster_;
  };
};
#endif
