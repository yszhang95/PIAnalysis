#include <iostream>

#include "PIFilterBase.hpp"
#include "PIAnaEvtBase.hpp"

#include "TChain.h"
#include "TError.h"
#include "TFile.h"
#include "TClonesArray.h"

#include "PIMCInfo.hh"

PIAnaEvtBase::PIAnaEvtBase(const std::string &treename)
    : treename_(treename), info_(nullptr), atar_(nullptr), track_(nullptr),
      out_fname_("output.root"), m_pion_(0.13957E3),
      m_kaon_(0.49367700E3), m_proton_(0.93827200E3),
      initialized_(false), verbose_(false)
{
  chain_ = std::make_unique<TChain>(treename.c_str());

  info_ = new PIMCInfo();
  atar_ = new TClonesArray("PIMCAtar");
  track_ = new TClonesArray("PIMCTrack");
  decay_ = new TClonesArray("PIMCDecay");

  chain_->SetBranchAddress("info", &info_);
  chain_->SetBranchAddress("atar", &atar_);
  chain_->SetBranchAddress("track", &track_);
  chain_->SetBranchAddress("decay", &decay_);
}

PIAnaEvtBase::~PIAnaEvtBase()
{
  if (!initialized_) {
    std::cerr
      << "[ERROR] Event Analyzer is never initialized until deletion.\n";
  }
  delete info_;
  delete atar_;
  delete track_;
  delete decay_;
}

void PIAnaEvtBase::out_file(const std::string &oname)
{
  if (!initialized_) {
    out_fname_ = oname;
  } else {
    std::cerr << "[ERROR] " << oname
              << " is not set as the name of output."
      " Please set output name before calling initialize().\n";
  }
}

void PIAnaEvtBase::add_file(const std::string& f)
{
  if (!initialized_) {
    filenames_.push_back(f);
  } else {
    std::cerr << "[ERROR] " << f
              << " is not added as input file."
      " Please add input file before calling initialize().\n";
  }
}

void PIAnaEvtBase::add_friend(const std::string& ftree)
{
  if (!initialized_) {
    ftreenames_.push_back(ftree);
  } else {
    std::cerr << "[ERROR] " << ftree
              << " is not added as friend tree."
      " Please add friend tree before calling initialize().\n";
  }
}

void PIAnaEvtBase::filter(const std::string &filterstr) {
  if (initialized_) {
    std::cerr << "[ERROR] " << filterstr
              << " is not set. PIAnaEvtBase::filter()"
                 " must be called before initialization.\n";
    return;
  }
  int run = -1, event = -1, eventid = -1;
  std::string::size_type i = 0;
  std::string::size_type j = filterstr.find(":");
  if (j == std::string::npos) {
    return;
  };
  run = std::stoi(filterstr.substr(i, j));
  i = j + 1;
  j = filterstr.find(":", i);
  if (j == std::string::npos) {
    return;
  };
  event = std::stoi(filterstr.substr(i, j));
  i = j+1;
  eventid = std::stoi(filterstr.substr(i, filterstr.size()));

  select_event_id_.event_id(run, event, eventid);
  std::cout << "[INFO] Setting event ID as " << run << ":"
              << event << ":" << eventid << ".\n";
}

void PIAnaEvtBase::add_filter(const std::string &n,
                              std::unique_ptr<PIFilterBase> f)
{
  if (initialized_) {
    const std::string msg =
      ::Form("Cannot add filter %s"
             " because the analyze has been initialized."
             " Call add_filter() before initialize().",
             n.c_str());
    Warning("PIAnaEvtBase::add_filter", msg.c_str());
    return;
  }
  if (filters_.find(n) != filters_.end()) {
    const std::string msg = ::Form("Override filter %s", n.c_str());
    Warning("PIAnaEvtBase::add_filter", msg.c_str());
    return;
  } else {
    const std::string msg = ::Form("Added filter %s", n.c_str());
    Info("PIAnaEvtBase::add_filter", msg.c_str());
  }
  filters_.insert({n, std::move(f)});
  filter_names_.push_back(n);
  return;
}

void PIAnaEvtBase::initialize() {
  initialized_ = true;
  for (const auto& ftname : ftreenames_) {
    chain_->AddFriend(ftname.c_str());
  }

  for (const auto& fname : filenames_) {
    chain_->Add(fname.c_str());
  }

  fout_ = std::make_unique<TFile>(out_fname_.c_str(), "recreate");
}
