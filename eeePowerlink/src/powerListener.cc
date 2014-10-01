#include "powerListener.h"

powerListener::powerListener( bool EEE ) {
    if ( EEE )
        powerVector.setName("EEE-Link Power");
    else
        powerVector.setName("NoEEE-Link Power");

    totPower = 0;
}

void powerListener::receiveSignal( cComponent *src, simsignal_t id, double d ) {
    EV << "Received signal " << id << " from " << src->getFullPath() << " value " << d << endl;
    powerVector.record(totPower);
    totPower += d;
    powerVector.record(totPower);
}

void powerListener::finish() {
    powerVector.~cOutVector();
}
