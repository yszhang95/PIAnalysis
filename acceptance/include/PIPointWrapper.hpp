#ifndef __PI_PointWrapper__
#define __PI_PointWrapper__

#include "Math/Point3D.h"

namespace PIAna
{
  template<typename T>
  class PointWrapper
  {
  public:
    PointWrapper() : isnull_(true) {}
    PointWrapper(const T &p) : p_(p), isnull_(false) {}
    PointWrapper(const PointWrapper<T> &p) {
      this->p_ = p.p_;
      isnull_ = p.isnull_;
    }
    PointWrapper<T> &operator=(const PointWrapper<T> &p) {
      this->p_ = p.p_;
      isnull_ = p.isnull_;
      return *this;
    }
    PointWrapper<T> &operator=(const T &p) {
      p_ = p;
      isnull_ = false;
      return *this;
    }
    bool null() { return isnull_; }
    const T &point() const { return p_; }

  private:
    T p_;
    bool isnull_;
  };

  using XYZPointWrapper = PointWrapper<ROOT::Math::XYZPoint>;

};
#endif
