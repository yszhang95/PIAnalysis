#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "PIEventAction.hpp"
#include "PIEventFilter.hpp"
#include "PIEventAnalyzer.hpp"
#include "PIEventProducer.hpp"
#include "PIJobManager.hpp"

#include "PIEventAction.hpp"
#include "TChain.h"
#include "TError.h"
#include "TFile.h"
#include "TClonesArray.h"

#include "PIMCInfo.hh"

PIAna::PIJobManager::PIJobManager()
    : treename_("sim"), info_(nullptr), atar_(nullptr), track_(nullptr),
      out_fname_("output.root"),
      initialized_(false)
{
  chain_ = std::make_unique<TChain>(treename_.c_str());

  info_ = new PIMCInfo();
  atar_ = new TClonesArray("PIMCAtar");
  track_ = new TClonesArray("PIMCTrack");
  decay_ = new TClonesArray("PIMCDecay");

  chain_->SetBranchAddress("info", &info_);
  chain_->SetBranchAddress("atar", &atar_);
  chain_->SetBranchAddress("track", &track_);
  chain_->SetBranchAddress("decay", &decay_);
}

PIAna::PIJobManager::~PIJobManager()
{
  if (!initialized_) {
    Error("PIAna::PIJobManger", "[ERROR] Job Manager is never initialized until deletion.");
  }
  delete info_;
  delete atar_;
  delete track_;
  delete decay_;
}


void PIAna::PIJobManager::begin()
{
  initialize();
  for (const auto &n : action_names_) {
    actions_.at(n)->Begin();
  }
}

void PIAna::PIJobManager::run()
{
  if (!initialized_) {
    std::cerr << "[ERROR] PIAnalyzer: Not initialized. "
                 "Call PIAnalyzer::begin() before PIAnalyzer::run()\n";
    return;
  }
  for (long long ientry = 0; ientry < chain_->GetEntriesFast(); ++ientry) {

    if (ientry % 1000 == 0) {
      std::cout << "Analyzing " << ientry << " events" << std::endl;
    }

    auto itree = chain_->LoadTree(ientry);
    if (itree < 0)
      break;

    auto bytes = chain_->GetEntry(ientry);
    if (bytes <= 0)
      break;

    data_.Put<const PIMCInfo *>("info", info_);
    data_.Put<const TClonesArray *>("atar", atar_);
    data_.Put<const TClonesArray *>("track", track_);
    data_.Put<const TClonesArray *>("decay", decay_);
    // check if analyzers are at the end of actions
    std::vector<int> is_analyzer;
    for (const auto &n : action_names_) {
      if (dynamic_cast<PIAna::PIEventAnalyzer *>(actions_.at(n).get())) {
        is_analyzer.push_back(1);
      } else {
        is_analyzer.push_back(0);
      }
    }
    // analyzers labeled by 1 must be at the end
    auto passed = std::is_sorted(is_analyzer.begin(), is_analyzer.end());
    if (!passed) {
      Error("PIAna::PIJobManager", "Event analyzers must be put after all filters.");
      std::abort();
    }
    do_actions();
  }
}

void PIAna::PIJobManager::do_actions()
{
  for (const auto &n : action_names_) {
    auto filter_ptr = dynamic_cast<PIAna::PIEventFilter *>(actions_.at(n).get());
    actions_.at(n)->DoAction(data_);
    if (filter_ptr) {
      const auto name = filter_ptr->GetName();
      const auto passed = data_.Get<bool>(name);
      if (!passed)
        return;
    }
  }
}

void PIAna::PIJobManager::end()
{
  for (const auto &n : action_names_) {
    actions_.at(n)->End();
  }
  fout_->Write();
}

void PIAna::PIJobManager::out_file(const std::string &oname)
{
  if (!initialized_) {
    out_fname_ = oname;
  } else {
    std::ostringstream msgstream;
    msgstream <<  oname
              << " is not set as the name of output."
                 " Please set output name before calling initialize().";
    Warning("PIAna::PIJobManager::out_file", msgstream.str().c_str());
  }
}

void PIAna::PIJobManager::add_file(const std::string& f)
{
  if (!initialized_) {
    filenames_.push_back(f);
  } else {
    std::ostringstream msgstream;
    msgstream << f
              << " is not added as input file."
                 " Please add input file before calling initialize().";
    Warning("PIAna::PIJobManager::add_file", msgstream.str().c_str());
  }
}

void PIAna::PIJobManager::add_friend(const std::string& ftree)
{
  if (!initialized_) {
    ftreenames_.push_back(ftree);
  } else {
    std::ostringstream msgstream;
    msgstream << ftree << " is not added as friend tree."
      " Please add friend tree before calling initialize().";
    Warning("PIAna::PIJobManager::add_friend", msgstream.str().c_str());
  }
}

void PIAna::PIJobManager::treename(const std::string &ftree)
{
  if (!initialized_) {
    treename_ = ftree;
  } else {
    std::ostringstream msgstream;
    msgstream << ftree << " is not set as the input tree."
      " Please set the input tree before calling initialize().";
    Warning("PIAna::PIJobManager::treename", msgstream.str().c_str());
  }
}

void PIAna::PIJobManager::add_action(const std::string &n,
                              std::unique_ptr<PIEventAction> action)
{
  if (initialized_) {
    const std::string msg =
      ::Form("Cannot add action %s"
             " because the Job Manager has been initialized."
             " Call add_action() before initialize().",
             n.c_str());
    Warning("PIAna::PIJobManager::add_action", msg.c_str());
    return;
  }
  if (actions_.find(n) != actions_.end()) {
    const std::string msg = ::Form("Override action %s", n.c_str());
    Warning("PIAna::PIJobManager::add_action", msg.c_str());
    actions_.at(n) = std::move(action);
  } else {
    const std::string msg = ::Form("Added action %s", n.c_str());
    Info("PIAna::PIJobManager::add_action", msg.c_str());
    actions_.insert({n, std::move(action)});
    action_names_.push_back(n);
  }
  actions_.at(n)->SetJobManager(this);
  return;
}

PIAna::PIEventAction *PIAna::PIJobManager::get_action(const std::string &n)
{
  if (actions_.find(n) == actions_.end()) {
    return nullptr;
  }
  return actions_.at(n).get();
}

void PIAna::PIJobManager::initialize() {
  initialized_ = true;
  for (const auto& ftname : ftreenames_) {
    chain_->AddFriend(ftname.c_str());
  }

  for (const auto& fname : filenames_) {
    chain_->Add(fname.c_str());
  }

  fout_ = std::make_unique<TFile>(out_fname_.c_str(), "recreate");
}
