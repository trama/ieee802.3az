#ifndef _EPL_CN_INTEL_H
#define _EPL_CN_INTEL_H

#include "EPLDefs.h"
#include "MACAddress.h"
#include "EPLframe_m.h"

#define MAX_REPLY_CHUNK_SIZE   1497

class EPL_ControlledNode_intel : public cSimpleModule, protected cListener {

    protected:
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
        cMessage *presTimer;
        simtime_t IStartTime;
        simtime_t IEndTime;
        simtime_t TPreq_Pres_CN;

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
        virtual void sendPacket( EPLframe *datapacket, const MACAddress& destAddr );
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, const char *s);
        virtual MACAddress setMACAddress(int);
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
