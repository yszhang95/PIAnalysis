#ifndef __PI_AnaCluster__
#define __PI_AnaCluster__

#include <algorithm>
#include <memory>
#include <vector>

#include "PIAnaHit.hpp"
#include "PIAnaGraph.hpp"
#include "PIAnaPointCloud.hpp"

/**
 * PIAnaCluster
 */
class PILocCluster
{
public:

  explicit PILocCluster() {};
  virtual ~PILocCluster() {};

  /** Set search radius */
  void radius(const double radius) { radius_ = radius; }
  /** Get search radius */
  double radius() const { return radius_; }

  /**
   * The method to cluster hits.
   * @return A map between reference points of each cluster and their
   * connected hits.
   */
  template <typename InputIter>
  std::map<const PIAnaHit *, std::vector<const PIAnaHit *>>
  cluster_hits(InputIter first, InputIter last)
  {
    std::vector<const PIAnaHit *> hits(first, last);
    return cluster_all_hits(hits);
  }

protected:
  /**
   * @return A map between reference/start hit and its connected hits.
   * \note{Objects in std::map are ordered by pointers' addresses not
   * those which they points to.}
   * \note{Reference point is defined in the protected member function
   * connected_hits() of the derived class}
   * \note{Reference point, as the key, can be nullptr and invalid. The
   * meaning of nullptr varies depending on the contexts and
   * implementations.}
   */
  virtual std::map<const PIAnaHit*, std::vector<const PIAnaHit *> >
  connected_hits(const std::vector<const PIAnaHit *> &) = 0;

  /**
   * @return A map between reference/start hit and its connected hits.
   * \note{Unlike connected_hits(), this method cluster all input hits
   * into their own collections.}
   */
  virtual std::map<const PIAnaHit*, std::vector<const PIAnaHit *> >
  cluster_all_hits(const std::vector<const PIAnaHit *> &) = 0;

  std::unique_ptr<PIAnaGraph> graph_;
  double radius_;
};

class PITCluster : public PILocCluster
{
public:
  explicit PITCluster();
  ~PITCluster();
  void t0(const double t0) { t0_ = t0; }
  double t0() const { return t0_; }

protected:
  std::map<const PIAnaHit *, std::vector<const PIAnaHit * > >
  connected_hits(const std::vector<const PIAnaHit *> &) final;
  std::map<const PIAnaHit *, std::vector<const PIAnaHit * > >
  cluster_all_hits(const std::vector<const PIAnaHit *> &) final;

private:
  double t0_;
};

class PIXYZCluster : public PILocCluster
{};

#endif
