#ifndef _SWITCHEEE_H
#define _SWITCHEEE_H

#include "EtherFrame_m.h"
#include "EPLDefs.h"
#include "EtherMACFullDuplex.h"

class internalHub_EEE : public cSimpleModule, protected cListener {
  private:
    double power;
    EtherMACFullDuplex *myMAC;

  protected:

    //FLAGS
    bool debug;
    short *gateLPI; //0 = no, 1 = LPI, -1 = triggered LPI
    bool dataratesDiffer;
    int countGates;
    bool partialLPI;
    int isStopLPI;
    int countGatesinLPI;

    // PARAMS
    int numHUB;
    int numPorts;
    bool terminator;
    int inputGateBaseId;  // gate id of ethg$i[0]
    int outputGateBaseId; // gate id of ethg$o[0]

    // POWER PARAMS
    double powerConsumptionEEE;
    double powerConsumption;
    double pEeePowerIncrement;
    double noEeePowerIncrement;
    //double noEeePowerDecrement;
    double eeePowerIncrement;
    //double eeePowerDecrement;

    // TIMERS
    simtime_t sleepTime, wakeTime, quietTime, refreshTime, eplCycleTime;
    simtime_t *IStartTime;
    simtime_t *IEndTime;
    simtime_t LPITime;
    simsignal_t noLpi_powerSignal;

    cMessage *stopIdleMessagePrototype;
    cMessage *timerMsg, *wakeUpTimer, *startLpiTimer;
    const char *MNaddress;
    char name[30];
    double percH;


    bool hasSubnetworks;
    // ADDRESS MANAGEMENT
    // address table as in Inet switch implementation
    struct MAC_compare
    {
        bool operator()(const MACAddress& u1, const MACAddress& u2) const
            {return u1.compareTo(u2) < 0;}
    };
    typedef std::map<MACAddress, int, MAC_compare> AddressTable;
    AddressTable addresstable;  // Address Lookup Table

    // STATS & SIGNALS
    long numMessages;   // number of messages handled
    simsignal_t pkSignal;
    simsignal_t powerSignal;
    simsignal_t rcvdPwNoEEESignal;

    // METHODS

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const {return 2;}
    virtual void handleMessage(cMessage *msg);
    virtual void handleSelfMsg(cMessage *msg);
    virtual void handleLowerMsg(cMessage *msg);
    virtual void manageLpiSignals(cMessage *msg);
    virtual void finish();


  private:
    EtherFrame *tmp;
    virtual void goLPI(int);
    virtual void wakeUp(int);
    virtual void checkConnections(bool errorWhenAsymmetric);
    virtual void handleAndDispatchFrame(EtherFrame*, cGate *g);
    virtual void broadcastFrame(EtherFrame *frame, cGate *g);
    virtual void printAddressTable();
    virtual int getPortForAddress(MACAddress& address);
    virtual void initAddressTable(MACAddress address);
    virtual void processFrame(cMessage *msg);
    virtual int isMulticast(MACAddress&);
    virtual EtherMACFullDuplex* getMyInterface();

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

