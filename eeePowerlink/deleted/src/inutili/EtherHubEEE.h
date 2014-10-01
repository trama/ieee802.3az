
#ifndef _ETHERHUBEEE_H
#define _ETHERHUBEEE_H

#include "INETDefs.h"

/**
 * Models a wiring hub. It simply broadcasts the received message
 * on all other ports.
 */
class EtherHubEEE : public cSimpleModule, protected cListener
{
  private:
    double power;
  protected:
    bool *gateLPI;
    simtime_t *IStartTime;
    simtime_t *IEndTime;
    simtime_t LPITime;
    simtime_t TPowCycle;
    int numHUB;
    int numPorts;         // sizeof(ethg)
    int countGates;
    int inputGateBaseId;  // gate id of ethg$i[0]
    int outputGateBaseId; // gate id of ethg$o[0]
    bool dataratesDiffer;
    double DefEEEPower;
    double DefNoEEEPower;
    double HNoEEEPower;
    double LNoEEEPower;
    double IPower;
    double HEEEPower;
    double LEEEPower;
    double L_TX_EEEPower;
    double L_RX_EEEPower;
    double L_TXRX_EEEPower;
    double L_RXTX_EEEPower;

    simtime_t Ts;
    simtime_t Tq;
    simtime_t Tw;

    // statistics
    long numMessages;   // number of messages handled
    cMessage *timerMsg, *wakeUpTimer, *startLpiTimer;
    static simsignal_t pkSignal;
    static simsignal_t rcvdPwSignal;
    static simsignal_t rcvdPwNoEEESignal;



  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void handleSelfMsg(cMessage *msg);
    virtual void handleLowerMsg(cMessage *msg);
    virtual void finish();
    virtual void goLPI();
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
    virtual void wakeUp();
    virtual void checkConnections(bool errorWhenAsymmetric);
};

#endif

