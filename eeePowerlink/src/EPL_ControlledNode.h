#ifndef _EPL_CN_H
#define _EPL_CN_H

#include "EPLDefs.h"
#include "MACAddress.h"
#include "EPLframe_m.h"

#define MAX_REPLY_CHUNK_SIZE   1497

class EPL_ControlledNode : public cSimpleModule, protected cListener {
    private:
        //double power;
        cOutVector powerVector;

    protected:
        int localSAP;
        int remoteSAP;
        char SoAString[30];
        int numHost;
        bool lpiOnSoA;
        bool hostAwake;
        bool debug;
        bool autoSetMACaddress;
        bool partialLPI;
        bool onlyRx;

        long seqNum;
        int reqLength;

        MACAddress destMACAddress;
        MACAddress srcAddr;

        // self messages
        cMessage *quietTimer, *wakeUpTimer, *awakeTimer, *presTimer, *lpiTimer;
        simtime_t sleepTime;
        simtime_t quietTime;
        simtime_t wakeTime, partialWakeTime;
        simtime_t eplCycleTime;
        simtime_t IStartTime;
        simtime_t IEndTime;
        simtime_t TPreq_Pres_CN;
        simtime_t propagationTime;

        // statistics
        long packetsSent;
        long packetsReceived;
        simsignal_t sentPkSignal;
        simsignal_t rcvdPkSignal;

        EPLframe *datapacket;

        MACAddress myMACaddress;
        int subNetIndex;

    protected:
        virtual void initialize();
        virtual void handleMessage( cMessage *msg );

        virtual void finish();
        virtual void registerDSAP( int dsap );
        virtual void sendLPISignal(int flag);
        virtual void sendPacket( EPLframe *datapacket, const MACAddress& destAddr );
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, const char *s);
        virtual MACAddress setMACAddress(int);
        //virtual void exitPLPI();
    private:
        virtual void handleSelfMsg( cMessage *msg );
        std::string logName( void ) {
            std::ostringstream ost;
            ost << this->getParentModule()->getName() << "::" << this->getFullName();
            if ( this->isVector() ) ost << "[" << this->getIndex() << "]";
            return ost.str();
        }
};

#endif
