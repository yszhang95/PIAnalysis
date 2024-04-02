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
    virtual void DoAction(PIEventData &evt);
    virtual void End();

    int GetCode() const { return code_; }
    const std::string GetName() const { return name_; }

    void Filter(bool use = true) { use_ = use; };

  protected:
    /**
     * @return true if pass the selection.
     */
    virtual bool get_bit(const PIEventData &evt) = 0;

  private:
    const std::string name_;
    const int code_;
    bool use_;
  };
};
#endif
