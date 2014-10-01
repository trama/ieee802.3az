/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>

#include "ControlledNodeSrv_AfterFrame.h"

#include "EtherApp_m.h"
#include "Ieee802Ctrl_m.h"
#include "EPLframe_m.h"
#include "powerListener.h"


Define_Module(ControlledNodeSrv_AfterFrame);

simsignal_t ControlledNodeSrv_AfterFrame::sentPkSignal = SIMSIGNAL_NULL;
simsignal_t ControlledNodeSrv_AfterFrame::rcvdPkSignal = SIMSIGNAL_NULL;
simsignal_t ControlledNodeSrv_AfterFrame::rcvdPwSignal = SIMSIGNAL_NULL;
simsignal_t ControlledNodeSrv_AfterFrame::rcvdPwNoEEESignal = SIMSIGNAL_NULL;
simsignal_t ControlledNodeSrv_AfterFrame::rcvdPw_1Signal = SIMSIGNAL_NULL;
simsignal_t ControlledNodeSrv_AfterFrame::rcvdPw_2Signal = SIMSIGNAL_NULL;

enum MNNumbers {
   // N = 14
    //N = 10
    N = 13
};

double perc2;

bool onSoa2 = false;
bool wakedUp2 = true;
double zero2 = 0;

ControlledNodeSrv_AfterFrame::ControlledNodeSrv_AfterFrame()
{
    timerMsg = NULL;
}
ControlledNodeSrv_AfterFrame::~ControlledNodeSrv_AfterFrame()
{
    cancelAndDelete(timerMsg);
}

void ControlledNodeSrv_AfterFrame::initialize()
{


    //powerVector.setName("Power");

    reqLength = &par("reqLength");
    respLength = &par("respLength");
    sendInterval = &par("sendInterval");

    localSAP = ETHERAPP_SRV_SAP;
    remoteSAP = ETHERAPP_CLI_SAP;

    seqNum = 0;
    WATCH(seqNum);

    // statistics
    packetsSent = packetsReceived = 0;
    sentPkSignal = registerSignal("sentPk");
    rcvdPkSignal = registerSignal("rcvdPk");
    rcvdPwSignal = registerSignal("rcvdPw");
    rcvdPwNoEEESignal = registerSignal("rcvdPwNoEEE");
    rcvdPw_1Signal = registerSignal("rcvdPw_1");
    rcvdPw_2Signal = registerSignal("rcvdPw_2");


    TPreq_Pres_CN = par("TPreq_Pres_CN");
    Ts = par("Ts"); // Time to go in LPI mode
    Tq = par("Tq"); // Refresh LPI signal period
    Tw = par("Tw"); // Wake up time
    Tplt = par("Tplt"); // Time to plot
    TPowCycle = par("TPowCycle"); // EPL cycle time
    TPropag = par("TPropag"); // propagation time
    EV << "tProp "<<TPropag << "ts " << Ts;
    DefEEEPower  = par("DefEEEPower"); // LPI Power consumption with EEE
    DefNoEEEPower  = par("DefNoEEEPower"); // LPI Power consumption without EEE

    HNoEEEPower  = par("HNoEEEPower"); // ACTIVE Power consumption increment
    LNoEEEPower  = par("LNoEEEPower"); // ACTIVE Power consumption increment
    HEEEPower  = par("HEEEPower"); // ACTIVE Power consumption increment
    LEEEPower  = par("LEEEPower"); // LPI Power consumption decrement
    L_TX_EEEPower  = par("L_TX_EEEPower"); // ACTIVE Power consumption increment
    L_RX_EEEPower  = par("L_RX_EEEPower"); // LPI Power consumption decrement
    L_TXRX_EEEPower  = par("L_TXRX_EEEPower"); // ACTIVE Power consumption increment
    L_RXTX_EEEPower  = par("L_RXTX_EEEPower"); // LPI Power consumption decrement
    IPower  = par("IPower"); // IDLE Power consumption

    EV<< "H" << HNoEEEPower << ", " << HEEEPower << "L" << LNoEEEPower << ", "<< LEEEPower << "\n";

    numHost = par("numHost"); // Number of the current host

    if(numHost == 1)
    {
        powerListener *listener = new powerListener(true);
        subscribe(rcvdPw_1Signal, listener);
    }
    else if(numHost == 2)
    {
        powerListener *listenerNoEEE = new powerListener(false);
        subscribe(rcvdPw_2Signal, listenerNoEEE);
    }

    sprintf(SoAString, "SoA%d", numHost);
    EV << "My number is `" << numHost << "'\n";

    WATCH(packetsSent);
    WATCH(packetsReceived);

    bool registerSAP = par("registerSAP");
    if (registerSAP)
        registerDSAP(localSAP);

    nextWake = 0;

    timerMsg = new cMessage("startPow",START_POW);
    scheduleAt(simTime(), timerMsg);

}

void ControlledNodeSrv_AfterFrame::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if(msg->getKind() == START_POW)
        {
            if(numHost == 1)
            {
                power = DefEEEPower;
                emit(rcvdPw_1Signal, power);
            }
            else if (numHost == 2)
            {
                power = DefNoEEEPower;
                emit(rcvdPw_2Signal, power);
            }

            power = DefEEEPower;
            emit(rcvdPwSignal, power);

            power = DefNoEEEPower;
            emit(rcvdPwNoEEESignal, power);

        }
        if(msg->getKind() == PLOT)
        {
            emit(rcvdPw_1Signal, zero2);
            emit(rcvdPw_2Signal, zero2);
            emit(rcvdPwSignal, zero2);
            emit(rcvdPwNoEEESignal, zero2);
        }
        if(msg->getKind() == WAKE_AFTER)
        {
            wakeUp();
        }
        if(msg->getKind() == SOA_GOLPI) // Go LPI after SOA
        {
            goLPI(); // Switch to LPI mode
        }
        if(msg->getKind() == START_LPI) // Start LPI mode
        {
//           simtime_t TqWait = simTime() + Tq;
//           timerMsg = new cMessage("quiet",QUIET);
//           scheduleAt(TqWait, timerMsg);

           if(numHost == 1)
           {
               power = L_TX_EEEPower;
               emit(rcvdPw_1Signal, power);
           }

           EV << "START LPI";
           power = L_TX_EEEPower;
           // EV << "Quiet, decremento potenza "<< power << "\n";
           emit(rcvdPwSignal, power);



           IStartTime = simTime();
           EV << "go LPI" << IStartTime << "\n";

        }
        if(msg->getKind() == START_LPITX) // Start LPI mode
        {
           simtime_t TqWait = simTime() + Tq;
           timerMsg = new cMessage("quiet",QUIET);
           scheduleAt(TqWait, timerMsg);
           if(numHost == 1)
           {
               power = L_TXRX_EEEPower;
               emit(rcvdPw_1Signal, power);
           }else if (numHost == 2)
           {
               power = LNoEEEPower;
               emit(rcvdPw_2Signal, power);
           }
           power = L_TXRX_EEEPower;
           emit(rcvdPwSignal, power);

           EV << "START LPI_TX";

           power = LNoEEEPower;
           // EV << "Quiet, decremento potenza "<< power << "\n";
           emit(rcvdPwNoEEESignal, power);

        }
        else if( ( msg->getKind() == QUIET ) && ( !wakedUp2 )) // Send LPI signals if the CN isn't waking up
        {
           sendLPISignal(); // Send LPI signal on the link
           EV << "Sending periodic LPI Signal\n";
           simtime_t TqWait = simTime() + Tq;
           timerMsg = new cMessage("quiet",QUIET);
           scheduleAt(TqWait, timerMsg);
           EV << "Quiet\n";
        }
        else if( msg->getKind() == WAKE_UP ) // Exit LPI mode
        {
           wakedUp2 = true;
           EV << "Waking up\n";
           IEndTime = simTime();
           EV << "exit LPI " << IEndTime << "\n";
           simtime_t TwWait = simTime() + Tw;
           timerMsg = new cMessage("wake", WAKED_UP);
           scheduleAt(TwWait, timerMsg);
        }
        else if( msg->getKind() ==  WAKED_UP ) // Change status in ACTIVE
        {
            if(numHost == 1)
            {
                power = HEEEPower;
                emit(rcvdPw_1Signal, power);
            }
            else if (numHost == 2)
            {
                power = HNoEEEPower;
                emit(rcvdPw_2Signal, power);
            }
            EV << "STOP LPI";

            power = HEEEPower;
            //EV << "ACTIVE, incremento potenza" << power<< "\n";
            emit(rcvdPwSignal, power);
            power = HNoEEEPower;
            //EV << "ACTIVE, incremento potenza" << power<< "\n";
            emit(rcvdPwNoEEESignal, power);
            perc2 = (IEndTime-IStartTime).dbl();
            EV << "Tempo trascorso in LPI dall'Host " <<  numHost << "Tidle = " << (IEndTime-IStartTime);
        }
        else if( msg->getKind() ==  SEND_PRES ) // Send PRes after TPreq_Pres_CN
        {
            EV << "sending PRES\n";
            sendPacket(datapacket, srcAddr);
            if( numHost != 3 && numHost != 4 && numHost != 5 && !onSoa2)
            {
                goLPI(); // Switch to LPI mode
            }
        }
    }
    else
    {
        EPLframe *req = check_and_cast<EPLframe *>(msg); // EPL frame received

        if( req->getType() == PREQ  )  // Preq received
        {
            //wakeUp();
            nextWake = simTime() + TPowCycle - Tw;
            timerMsg = new cMessage("WakeAfter",WAKE_AFTER);
            scheduleAt(nextWake, timerMsg);

            EV << "Received packet `" << msg->getName() << "'\n";

            packetsReceived++;
            emit(rcvdPkSignal, req);

            // PRes frame Generation

            Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(req->removeControlInfo());
            srcAddr = ctrl->getSrc();
            char msgname[30];
            strcpy(msgname, msg->getName());
            msgname[3] = 's';
            delete msg;
            delete ctrl;
            int k = 0;
            EV << "Generating packet `" << msgname << "'\n";
            datapacket = new EPLframe(msgname, IEEE802CTRL_DATA); // Pres frame

            long len = reqLength->longValue();
            datapacket->setByteLength(len);
            datapacket->setType(PRES);
            datapacket->setHost(numHost);
            simtime_t Tpres = simTime() + TPreq_Pres_CN;
            timerMsg = new cMessage("SendPres",SEND_PRES);
            scheduleAt(Tpres, timerMsg);
            k++;
        }
        else if (req->getType() == SOA)     // SoA received
        {
            goLPITX();
            if(req->getHost() == numHost){ // SoA indexed to me, send ASnd and switch to LPI mode

                EV << "Received packet `" << msg->getName() << "'\n";
                packetsReceived++;
                emit(rcvdPkSignal, req);

                // ASnd frame Generation

                Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(req->removeControlInfo());
                srcAddr = ctrl->getSrc();
                char msgname[30];
                sprintf(msgname, "AsyncSendHost%d", numHost);
                delete msg;
                delete ctrl;

                int k = 0;
                EV << "Generating packet `" << msgname << "'\n";
                datapacket = new EPLframe(msgname, IEEE802CTRL_DATA); // ASnd frame

                long len = reqLength->longValue();
                datapacket->setByteLength(len);
                datapacket->setType(ASYNC);
                sendPacket(datapacket, srcAddr);
                k++;
                simtime_t Tsoa = simTime() + TPropag;
                timerMsg = new cMessage("SoaGoLPI",SOA_GOLPI);
                scheduleAt(Tsoa, timerMsg);

            }
            else // SoA not indexed to me, switch to LPI mode
            {
                if( numHost == 3 || numHost == 4 || numHost == 5 || onSoa2)
                {
                    goLPI(); // Switch to LPI mode
                }
            }

        }
        else if (req->getType() == STOP_IDLE &&  nextWake == 0)  // wake up if stopIDLE received and it's our cycle
        {
            wakeUp();
        }
    }
}

void ControlledNodeSrv_AfterFrame::sendLPISignal()
{

        char msgname[30];
        sprintf(msgname, "LPISignal");
        EV << "Generating LPI packet `" << msgname << "'\n";

        // LPI signal Generation

        datapacket = new EPLframe(msgname, IEEE802CTRL_DATA);
        datapacket->setType(LPI_SIGNAL);

        //long len = reqLength->longValue();
        datapacket->setByteLength(0);

        Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
        etherctrl->setSsap(localSAP);
        etherctrl->setDsap(remoteSAP);
        datapacket->setControlInfo(etherctrl);

        emit(sentPkSignal, datapacket);
        send(datapacket, "out");
        packetsSent++;

}

void ControlledNodeSrv_AfterFrame::goLPI()  // Switch to LPI mode
{
    sendLPISignal();
    simtime_t TsWait = simTime() + Ts;
    timerMsg = new cMessage("startLPI", START_LPI);
    scheduleAt(TsWait, timerMsg);
    EV << "Starting Low Power Idle\n";
    wakedUp2 = false;
}
void ControlledNodeSrv_AfterFrame::goLPITX()  // Switch to LPI mode
{
    simtime_t TsWait = simTime() + Ts;
    timerMsg = new cMessage("startLPITX", START_LPITX);
    scheduleAt(TsWait, timerMsg);
    EV << "Starting Partial Low Power Idle\n";
}

void ControlledNodeSrv_AfterFrame::wakeUp() // Switch to ACTIVE mode
{
    cancelEvent(timerMsg);
    timerMsg = new cMessage("wakeUp", WAKE_UP);
    scheduleAt(simTime(), timerMsg);
}

void ControlledNodeSrv_AfterFrame::sendPacket(cPacket *datapacket, const MACAddress& destAddr)
{
    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setSsap(localSAP);
    etherctrl->setDsap(remoteSAP);
    etherctrl->setDest(destAddr);
    datapacket->setControlInfo(etherctrl);
    emit(sentPkSignal, datapacket);
    send(datapacket, "out");
    packetsSent++;
}

void ControlledNodeSrv_AfterFrame::registerDSAP(int dsap)
{
    EV << getFullPath() << " registering DSAP " << dsap << "\n";

    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setDsap(dsap);
    cMessage *msg = new cMessage("register_DSAP", IEEE802CTRL_REGISTER_DSAP);
    msg->setControlInfo(etherctrl);

    send(msg, "out");
}

void ControlledNodeSrv_AfterFrame::finish()
{
}
