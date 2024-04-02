#ifndef __PI_JobManager__
#define __PI_JobManager__

#include <TError.h>
#include <iostream>
#include <memory>
#include <map>
#include <string>
#include <vector>

#include "PIEventAction.hpp"
#include "PIEventData.hpp"

class TChain;
class TFile;
class TClonesArray;
class PIMCInfo;

namespace PIAna
{
  class PIJobManager
  {
  public:
    PIJobManager();
    ~PIJobManager();
    void begin();
    void run();
    void end();

    template <typename InputIter>
    void filenames(InputIter first, InputIter last) {
      if (!initialized_) {
        filenames_.assign(first, last);
      } else {
        Warning("PIAna::PIJobManger", "Input files are not added."
                " Please set input files before calling initialize().");
      }
    };
    void out_file(const std::string&);
    void add_file(const std::string&);
    void add_friend(const std::string&);
    void treename(const std::string&);

    void add_action(const std::string &n, std::unique_ptr<PIEventAction> action);

  private:
    void do_actions();

    void initialize();

    PIEventData data_;

    std::map<std::string, std::unique_ptr<PIEventAction>> actions_;
    std::vector<std::string> action_names_;
    std::unique_ptr<TChain> chain_;
    std::unique_ptr<TFile> fout_;

    PIMCInfo* info_;
    TClonesArray *track_;
    TClonesArray *atar_;
    TClonesArray *decay_;

    bool initialized_;

    std::string out_fname_;
    std::string treename_;
    std::vector<std::string> filenames_;
    std::vector<std::string> ftreenames_;
  };
};
#endif
