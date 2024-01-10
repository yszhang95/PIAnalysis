#ifndef __PI_ANA_EvtBase__
#define __PI_ANA_EvtBase__
#include <Rtypes.h>
#include <iostream>
#include <memory>
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

  void filter(const std::string&);

  ClassDef(PIAnaEvtBase, 1)

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

  select_event_id select_event_id_;
  std::unique_ptr<TChain> chain_;
  std::unique_ptr<TFile> fout_;

  PIMCInfo* info_;
  TClonesArray* track_;
  TClonesArray* atar_;

  bool initialized_;

private:
  std::string out_fname_;
  std::string treename_;
  std::vector<std::string> filenames_;
  std::vector<std::string> ftreenames_;

};

#endif
