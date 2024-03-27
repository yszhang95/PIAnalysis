#include "PITkFinder.hpp"

#include "TPrincipal.h"
#include <Math/Vector3D.h>
#include <Math/Vector3Dfwd.h>
#include <TError.h>
#include <TMatrixDUtilsfwd.h>

PIAna::PITkPCA::PITkPCA(int ndim) : fitted_(false) {
  // N: normalized
  // D: store data
  pca_ = new TPrincipal(ndim, "D");
  // std::cout << "Data are not normalized and assumed to be in the same magnitude.\n";
}

PIAna::PITkPCA::~PITkPCA() { delete pca_; };

void PIAna::PITkPCA::add_data(double* row)
{
  if (!fitted_) {
    pca_->AddRow(row);
  } else {
    Warning("PIAna::PITkPCA",
            "Data not added to PITkPCA because the PCA has been calculated. Please call clear_data()");
  }

}

void PIAna::PITkPCA::fit() {
  pca_->MakePrincipals();
  fitted_ = true;
}

ROOT::Math::Polar3DVector PIAna::PITkPCA::get_direction()
{
  const auto &eigens = *pca_->GetEigenVectors();
  const auto v0_view = TMatrixDColumn_const(eigens, 0);
  ROOT::Math::XYZVector xyz {
    v0_view(0), v0_view(1), v0_view(2)
  };
  return ROOT::Math::Polar3DVector{xyz}.Unit();
}

void PIAna::PITkPCA::clear_data()
{
  fitted_ = false;
  pca_->Clear("");
}
