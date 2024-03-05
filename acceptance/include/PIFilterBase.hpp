#ifndef __PI_FilterBase__
#define __PI_FilterBase__
#include "TObject.h"

class TH1D;
class TClonesArray;
class PIMCInfo;

class PIFilterBase
{
public:
  PIFilterBase(const int);
  virtual ~PIFilterBase();

  /**
   * @param info
   * @param track
   * @param atar
   * @param decay
   * \note{Must be called once and before eval_bit() per event.}
   */
  void load_event(PIMCInfo *info, TClonesArray *track, TClonesArray *atar, TClonesArray *decay);
  /**
   * @return true if pass the selection.
   * \note{Must be called once and after load_event() per event.}
   */
  bool eval_bit();

  int get_code() { return code_; };

protected:
  virtual bool get_bit() = 0;

  PIMCInfo* info_;
  TClonesArray *track_;
  TClonesArray *atar_;
  TClonesArray *decay_;

  const int code_;

  bool loaded_;

private:
};
#endif
