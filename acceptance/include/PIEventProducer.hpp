#ifndef __PI_EventProducer__
#define __PI_EventProducer__
#include <string>
#include "PIEventAction.hpp"
namespace PIAna
{
  class PIEventProducer : public PIEventAction
  {
  public:
    PIEventProducer(const std::string&);
    virtual ~PIEventProducer();

    virtual void Begin();
    virtual void DoAction(PIEventData &evt);
    virtual void End();

    const std::string GetName() const { return name_; }

  protected:
    /**
     * produce
     */
    virtual void produce(PIEventData &evt) = 0;

  private:
    const std::string name_;
  };
};
#endif
