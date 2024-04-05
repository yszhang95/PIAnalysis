#ifndef __PI_EvtNbFilter__
#define __PI_EvtNbFilter__
#include "PIEventFilter.hpp"
#include "PIEventData.hpp"

#include <string>
#include <vector>

namespace PIAna
{
  class  PIEvtNbFilter: public PIEventFilter
  {
  public:
    explicit PIEvtNbFilter(const std::string& n, const int code);
    ~PIEvtNbFilter() override;
    void Begin() override;
    void DoAction(PIEventData&) override;
    void End() override;

    void load_filter(const std::string &);

  protected:
    bool filter(const PIEventData&) override;

    struct select_event_id {
      struct event_info {
        int run;
        int event;
        int eventid;
        event_info(int run, int event, int eventid)
            : run(run), event(event), eventid(eventid) {}
      };
      std::vector<event_info> ids_;
      select_event_id() {}

      void load_event_id(const std::string &);

      bool operator()(const int run, const int event, const int eventid);
    };

  private:
    select_event_id selector_;
};
};

#endif
