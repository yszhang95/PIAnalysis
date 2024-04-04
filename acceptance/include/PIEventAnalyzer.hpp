#ifndef __PI_EventAnalyzer__
#define __PI_EventAnalyzer__
#include <string>
#include "PIEventAction.hpp"
namespace PIAna
{
  class PIEventAnalyzer : public PIEventAction
  {
  public:
    PIEventAnalyzer(const std::string&);
    virtual ~PIEventAnalyzer();

    virtual void Begin();
    virtual void DoAction(PIEventData &evt);
    virtual void End();

    const std::string GetName() const { return name_; }

  protected:
    /**
     * analyze
     */
    virtual void analyze(const PIEventData &evt) = 0;

  private:
    const std::string name_;
  };
};
#endif
