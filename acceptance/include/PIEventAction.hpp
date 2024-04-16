#ifndef __PI_EventAction__
#define __PI_EventAction__

namespace PIAna
{
  class PIEventData;
  class PIJobManager;
  class PIEventAction
  {
  public:
    PIEventAction() : initialized_(false), verbose_(false), mgr_(nullptr) {}
    virtual ~PIEventAction() {};
    virtual void Begin() = 0;
    virtual void DoAction(PIEventData& evt) = 0;
    virtual void End() = 0;

    void verbose(const bool verbose) { verbose_ = verbose; }

    void SetJobManager(PIJobManager* mgr) { mgr_ = mgr; }

  protected:
    virtual void report() = 0;

    void initialize() { initialized_ = true; };
    bool initialized_;
    bool verbose_;
    PIJobManager* mgr_;
  };
};
#endif
