/*********************************************************************
*                                                                    *
* 2024-04-09 15:54:16                                                *
* acceptance/include/PIAccAnalyzer.hpp                               *
*                                                                    *
*********************************************************************/

#ifndef __PI_AccAnalyzer__
#define __PI_AccAnalyzer__

#include "Rtypes.h"

#include "PIEventAnalyzer.hpp"

class TTree;
// class TNtuple;
class TH2D;
class TH1D;

namespace PIAna
{
  class PIAccAnalyzer : public PIEventAnalyzer
  {
  public:
    PIAccAnalyzer(const std::string&);
    ~PIAccAnalyzer();

    void Begin() override;
    void DoAction(PIEventData&) override;
    void End() override;

    // void e_topo_name(const std::string &n) { e_topo_name_ = n; }
    void pivertex_name(const std::string &n) { pivertex_name_ = n; }
    void estart_name(const std::string &n) { estart_name_ = n; }
    void true_pivertex_name(const std::string &n) { true_pivertex_name_ = n; }
    void true_estart_name(const std::string &n) { true_estart_name_ = n; }
    void topoflag_name(const std::string &n) { topoflag_name_ = n; }
    void edirection_name(const std::string &n) { edirection_name_ = n; }

  protected:
    void analyze(const PIEventData&) override;
    void report() override;

  private:
    void clear();

    // std::string e_topo_name_;
    std::string pivertex_name_;
    std::string estart_name_;
    std::string true_pivertex_name_;
    std::string true_estart_name_;
    std::string topoflag_name_;
    std::string edirection_name_;

    TTree* t_;
    TH2D *h_pistop_rec_xy_;
    TH2D *h_pistop_rec_xy_5hits_;
    TH2D *h_e_rec_angle_;
    Float_t pi_x_;
    Float_t pi_y_;
    Float_t pi_z_;
    Float_t pi_rec_x_;
    Float_t pi_rec_y_;
    Float_t pi_rec_z_;
    Float_t e_x_;
    Float_t e_y_;
    Float_t e_z_;
    Float_t e_rec_x_;
    Float_t e_rec_y_;
    Float_t e_rec_z_;
    Float_t e_rec_theta_;
    Float_t e_rec_phi_;
  };
};
#endif
