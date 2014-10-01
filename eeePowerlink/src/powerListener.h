
#include "INETDefs.h"


class powerListener : public cListener
{
private:
    cOutVector powerVector;
    double totPower;
public:
    powerListener(bool EEE);
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, double d);
    virtual void finish();
};
