#ifndef __PI_TkFinder__
#define __PI_TkFinder__

#include <Math/Vector3Dfwd.h>

class TPrincipal;
namespace PIAna{
  class PITkFinder
  {
  public:
    PITkFinder() {};
    virtual ~PITkFinder() {};
    virtual void add_data(double*) = 0;
    virtual void fit() = 0;
    virtual ROOT::Math::Polar3DVector get_direction() = 0;
  };

  class PITkPCA : public PITkFinder {
  public:
    PITkPCA(int ndim=3);
    ~PITkPCA() override;
    void add_data(double*) override;
    void fit() override;
    ROOT::Math::Polar3DVector get_direction() override;

    void clear_data();

  private:
    PITkPCA(const PITkPCA &other) = delete;
    PITkPCA& operator=(const PITkPCA& other)= delete;
    PITkPCA(PITkPCA &&other) = delete;
    PITkPCA&& operator=(PITkPCA &&other) = delete;
    TPrincipal *pca_;
    bool fitted_;
  };
};
#endif
