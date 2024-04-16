#ifndef __PI_EventFilter__
#define __PI_EventFilter__
#include "PIEventAction.hpp"
#include "string"
namespace PIAna
{
  class PIEventFilter : public PIEventAction
  {
  public:
    PIEventFilter(const std::string, const int code);
    virtual ~PIEventFilter();

    virtual void Begin();
    virtual void DoAction(PIEventData&);
    virtual void End();

    int GetCode() const { return code_; }
    const std::string GetName() const { return name_; }

    void Filter(bool use = true) { use_ = use; };

  protected:
    /**
     * @return true if pass the selection.
     */
    virtual bool filter(const PIEventData&) = 0;
    virtual void report() {}

  private:
    const std::string name_;
    const int code_;
    bool use_;
  };
};
#endif
