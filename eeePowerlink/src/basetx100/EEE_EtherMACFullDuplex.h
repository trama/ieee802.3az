//
// Copyright (C) 2013 Federico Tramarin
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __EEE_ETHER_DUPLEX_MAC_H
#define __EEE_ETHER_DUPLEX_MAC_H

#include "INETDefs.h"
#include "EPLDefs.h"
#include "EtherMACFullDuplex.h"

/**
 * A simplified version of EtherMAC. Since modern Ethernets typically
 * operate over duplex links where's no contention, the original CSMA/CD
 * algorithm is no longer needed. This simplified implementation doesn't
 * contain CSMA/CD, frames are just simply queued up and sent out one by one.
 */
class EEE_EtherMACFullDuplex : public EtherMACFullDuplex
{
  public:
    EEE_EtherMACFullDuplex();
    bool eee;
    bool debug;

  protected:
    enum EEE_MACTransmitState
    {
        TX_QUIET = 10,
        TX_ACTIVE,
        TX_TRANSITION
    };

    enum EEE_MACReceiveState
    {
        RX_QUIET = 20,
        RX_ACTIVE,
        RX_TRANSITION
    };

    EEE_states lpiState;
    //EEE_MACReceiveState lpiRxState;
    simtime_t sleepTime, wakeTime, quietTime, refreshTime, partialWakeTime, partialSleepTime;
    // self messages
    cMessage *wakeUpTimer, *wakedUpTimer, *quietTimer, *refreshTimer, *startLpiTimer;
    //
    int mac2lpiControl, lpi2macControl;
    virtual void initialize(int stage);
    virtual void initializeStatistics();
    virtual void initializeFlags();
    //virtual void handleMessage(cMessage *msg);

    // finish
    virtual void finish();

    // event handlers
    //virtual void handleEndTxPeriod();
    virtual void handleMessage(cMessage *msg);
    virtual void handleSelfMessage(cMessage *msg);
    virtual void handleLpi2MacControl(cMessage *msg);

    // helpers
    //virtual void startFrameTransmission();
    virtual void processFrameFromUpperLayer(EtherFrame *frame);
    //virtual void processMsgFromNetwork(EtherTraffic *msg);
    //virtual void processReceivedDataFrame(EtherFrame *frame);
    virtual void scheduleLpiWakeUp();
    virtual void beginSendFrames();
    void handleLinkWakedUp();

    // triggering of EEE operations
    virtual void receiveSignal(cComponent *src, simsignal_t signalId, long l);

    //communication between MAC and LPIClient
    simsignal_t macSignals;

    // statistics
    simtime_t totalSuccessfulRxTime; // total duration of successful transmissions on channel

    /*std::string logName( void ) {
        std::ostringstream ost;
        ost << this->getName();
        if ( this->isVector() ) ost << "[" << this->getIndex() << "]";

        return ost.str();
    }*/

    std::string logName( void ) {
        std::ostringstream ost;
        cModule *tmp = this->getParentModule();
        while(tmp->getParentModule()->getParentModule() != NULL)
            tmp = tmp->getParentModule();
        ost << tmp->getName();
        ost << "::" << this->getName();
        if ( this->isVector() ) ost << "[" << this->getIndex() << "]";
        return ost.str();
    }

};

#endif

