#ifndef __PI_AnaConst__
#define __PI_AnaConst__

namespace PIAna {
  enum class EvtCode {
    Pi = 0,
    PiDAR,
    PiDIF,
    PiDARNoFirstLayerHit,
    PiOffATAR,
    PiDARMergedT,
    PiWrongT
  };

  extern const double m_pi;
  extern const double m_k;
  extern const double m_p;
  extern const double m_mu;
  extern const double m_e;
  extern const int pip_pdgid; // pi+
  extern const int pim_pdgid; // pi-
  extern const int mup_pdgid; // mu+
  extern const int mum_pdgid; // mu-
  extern const int ep_pdgid;  // e+
  extern const int em_pdgid;  // e-
  extern const int gamma_pdgid; // gamma
};
#endif
