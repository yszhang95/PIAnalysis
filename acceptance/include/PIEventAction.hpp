#ifndef __PI_EventAction__
#define __PI_EventAction__

namespace PIAna
{
  class PIEventData;
  class PIJobManager;
  class PIEventAction
  {
  public:
    PIEventAction() : initialized_(false), mgr_(nullptr) {}
    virtual ~PIEventAction() {};
    virtual void Begin() = 0;
    virtual void DoAction(PIEventData& evt) = 0;
    virtual void End() = 0;

    void SetJobManager(PIJobManager* mgr) { mgr_ = mgr; }
    // for filter, producer, and analyzer to read in-memory data holder
    // virtual void Read() = 0;
    // for producer write to in-memory data holder
    // virtual void Update() {};
    // for analyzer write objects to TFile that is owned by job manager;
    // virtual void AddToFile() = 0;
  protected:
    void initialize() { initialized_ = true; };
    bool initialized_;
    PIJobManager* mgr_;
  };
};
#endif
