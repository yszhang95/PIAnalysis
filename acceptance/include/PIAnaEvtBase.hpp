#ifndef __PI_ANA_EvtBase__
#define __PI_ANA_EvtBase__
#include <Rtypes.h>
#include <iostream>
#include <memory>
#include <map>
#include <string>
#include <vector>

class TChain;
class TH1D;
class TTree;
class TFile;
class TClonesArray;

class PIAnaHit;
class PIAnaHitMerger;
class PIAnaG4StepDivider;

class PIMCInfo;

class PIFilterBase;

class PIAnaEvtBase
{
public:

  PIAnaEvtBase(const std::string&);
  virtual ~PIAnaEvtBase();
  /**
     Derived function must call initialize() at the beginning in
     begin(). Other initializations must be done in begin() as well.
   */
  virtual void begin() = 0;
  /**
     Event loop following begin() immediately.
   */
  virtual void run() = 0;
  /**
     Summary.
   */
  virtual void end() = 0;

  template <typename InputIter>
  void filenames(InputIter first, InputIter last) {
    if (!initialized_) {
      filenames_.assign(first, last);
    } else {
      std::cerr << "[ERROR] input files are not added."
        " Please set input files before calling initialize().\n";
    }
  };
  void out_file(const std::string&);
  void add_file(const std::string&);
  void add_friend(const std::string &);

  void filter(const std::string &);
  void add_filter(const std::string&, std::unique_ptr<PIFilterBase>);

  void verbose(const bool verbose) { verbose_ = verbose; }

protected :
  struct select_event_id {
    int run;
    int event;
    int eventid;
    select_event_id() : run(-1), event(-1), eventid(-1) {}
    void event_id(int run, int event, int eventid)
    {
      this->run = run;
      this->event = event;
      this->eventid = eventid;
    }
    bool operator()(int run, int event, int eventid)
    {
      return (this->run == run && this->event == event
              && this->eventid == eventid)
      || this->run < 0 || this->event<0 || this->eventid < 0;
    }
  };
  void initialize();

  std::map<std::string, std::unique_ptr<PIFilterBase>> filters_;
  std::vector<std::string> filter_names_;

  select_event_id select_event_id_;
  std::unique_ptr<TChain> chain_;
  std::unique_ptr<TFile> fout_;

  PIMCInfo* info_;
  TClonesArray *track_;
  TClonesArray *atar_;
  TClonesArray *decay_;

  const double m_pion_; // MeV
  const double m_kaon_; // MeV
  const double m_proton_; // MeV

  bool initialized_;

  bool verbose_;

private:
  std::string out_fname_;
  std::string treename_;
  std::vector<std::string> filenames_;
  std::vector<std::string> ftreenames_;

};

#endif
