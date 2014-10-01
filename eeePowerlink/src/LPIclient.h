
#ifndef LPICLIENT_H_
#define LPICLIENT_H_

#include "EPLframe_m.h"
#include "INETDefs.h"
#include "EPLDefs.h"

class LPI_client : public cSimpleModule {
    protected:
        virtual void initialize( int stage );
        virtual int numInitStages() const {
            return 2;
        }
        virtual void handleSelfMsg( cMessage *msg );
        virtual void handleLowerMsg( cMessage *msg );
        virtual void handleUpperMsg( cMessage *msg );
        virtual void handleUpperCtrl( cMessage *msg );
        virtual void finish();

        virtual void handleMessage( cMessage *msg ) {
            if ( msg->isSelfMessage() ) {
                handleSelfMsg(msg);
            }
            else {
                if ( msg->getArrivalGateId() == lowerLayerIn ) {
                    EVT << "Msg from down. Sending up." << endl;
                    handleLowerMsg(msg);
                }
                else if ( msg->getArrivalGateId() == upperLayerIn ) {
                    EVT << "Msg from up." << endl;
                    handleUpperMsg(msg);
                }
                else if ( msg->getArrivalGateId() == upperControlIn ) {
                    EVT << "Ctrl msg from up." << endl;
                    handleUpperCtrl(msg);
                }
                else if ( msg->getArrivalGateId() == -1 ) {
                    opp_error("No self message and no gateID?? Check configuration.");
                }
                else {
                    opp_error("Unknown gateID?? Check configuration or override handleMessage().");
                }
            }
        }

        void sendDown( cMessage *msg );
        void sendCtrlUp( cMessage *msg );
        void sendUp( cMessage *msg );
        void wakeUp(EPLframe *frame );
        void fullActive(EPLframe *frame );

    private:

        simtime_t sleepTime, wakeTime, quietTime, refreshTime, partialWakeTime, partialSleepTime;
        // self messages
        cMessage *wakeUpTimer, *wakedUpTimer, *quietTimer, *refreshTimer, *startLpiTimer;
        int upperLayerIn;
        int upperLayerOut;
        int upperControlIn;
        int upperControlOut;
        int lowerLayerIn;
        int lowerLayerOut;
        bool debug;
        bool active;
        bool partialLPI;
        bool thisMN;

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

        cQueue *queue;

        std::string logName( void ) {
            std::ostringstream ost;
            ost << this->getName();
            if ( this->isVector() ) ost << "[" << this->getIndex() << "]";

            return ost.str();
        }
};

#endif /* LPICLIENT_H_ */
