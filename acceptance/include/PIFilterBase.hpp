#ifndef __PI_FilterBase__
#define __PI_FilterBase__

class TClonesArray;
class PIMCInfo;

class PIFilterBase
{
public:
  PIFilterBase();
  virtual ~PIFilterBase();

  /**
   * \note{Must be called once and before eval_bit() per event.}
   */
  void load_event(PIMCInfo *, TClonesArray *, TClonesArray *, TClonesArray *);
  /**
   * @return true if pass the selection.
   * \note{Must be called once and after load_event() per event.}
   */
  bool eval_bit();

protected:
  virtual bool get_bit() = 0;

  PIMCInfo* info_;
  TClonesArray *track_;
  TClonesArray *atar_;
  TClonesArray *decay_;
  bool loaded_;

private:
};
#endif
