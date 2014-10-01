
#ifndef LPICLIENT_100BASETX_H_
#define LPICLIENT_100BASETX_H_

//#include "EPLframe_m.h"
#include "EPLDefs.h"
#include "powerListener.h"

class LPI_client_100baseTX : public cSimpleModule, cListener {
    protected:
        virtual void initialize( int stage );
        virtual int numInitStages() const {
            return 2;
        }
        virtual void handleSelfMsg( cMessage *msg );
        //virtual void handleLowerMsg( cMessage *msg );
        //virtual void handleUpperMsg( cMessage *msg );
        virtual void handleMac2LpiControl( cMessage *msg );
        //virtual void handleLpiCtrlOut( cMessage *msg );
        virtual void finish();

        virtual void findMyNeighbour(bool request, cMessage *msg);

        virtual void handleMessage( cMessage *msg ) {
            if ( msg->isSelfMessage() ) {
                handleSelfMsg(msg);
            }
            else {
                if ( msg->getArrivalGateId() == mac2lpiControl ) {
                    EVT << "Msg from MAC." << endl;
                    handleMac2LpiControl(msg);
                }
//                else if ( msg->getArrivalGateId() == upperLayerIn ) {
//                    EVT << "Msg from up." << endl;
//                    //handleUpperMsg(msg);
//                }
//                else if ( msg->getArrivalGateId() == upperControlIn ) {
//                    EVT << "Ctrl msg from up." << endl;
//                    //handleUpperCtrl(msg);
//                }
                else if ( msg->getArrivalGateId() == -1 ) {
                    opp_error("No self message and no gateID?? Check configuration.");
                }
                else {
                    opp_error("Unknown gateID?? Check configuration or override handleMessage().");
                }
            }
        }

        void wakeUp(cMessage *frame );
        void fullActive(cMessage *frame );

        // triggering of EEE operations
        virtual void receiveSignal(cComponent *src, simsignal_t signalId, long l);
        virtual void receiveSignal(cComponent *src, simsignal_t signalId, const char *s);
    private:

        simtime_t sleepTime, wakeTime, quietTime, refreshTime, partialWakeTime, partialSleepTime;
        // self messages
        cMessage *wakeUpTimer, *wakedUpTimer, *quietTimer, *refreshTimer, *startLpiTimer, *lpiMessage;
//        int upperLayerIn;
//        int upperLayerOut;
        int lpi2macControl;
        int mac2lpiControl;
//        int lowerLayerIn;
//        int lowerLayerOut;
        bool debug;
        bool active;
        bool partialLPI;

        int neighborLPIClient;
        //bool thisMN;

        powerListener *eee_listener, *std_listener;

        simtime_t IStartTime;
        simtime_t IEndTime;
        simtime_t PIStartTime;
        simtime_t PITotTime;

        double totPower, lpiPower, noEeePowerIncrement, eeePowerIncrement, eeeTransitionPower;
        double pLpiPower, pEeePowerIncrement, pEeeTransitionPower;

        simsignal_t powerSignal;
        simsignal_t lpiSignal, partial_lpi_time;
        simsignal_t lpi_time;
        simsignal_t noLpi_powerSignal;

        //communication between MAC and LPIClient
        simsignal_t macSignals;

        cQueue *queue;

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
