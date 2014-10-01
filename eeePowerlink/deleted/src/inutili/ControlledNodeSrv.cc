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

#include "ControlledNodeSrv.h"

#include "EtherApp_m.h"
#include "Ieee802Ctrl_m.h"
#include "EPLframe_m.h"
#include "powerListener.h"


Define_Module(ControlledNodeSrv);

void ControlledNodeSrv::initialize()
{

    reqLength = &par("reqLength");
    respLength = &par("respLength");
    sendInterval = &par("sendInterval");

    onSoa = par("onSoA").boolValue();
    wakedUp = true;
    zero = 0;

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

    if(numHost == 1) // EEE in a single node
    {
        powerListener *listener = new powerListener(true);
        subscribe(rcvdPw_1Signal, listener);
    }
    else if(numHost == 2) // no EEE in a single node
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

    startLpiTimer = new cMessage("startLPI", START_LPI);
    startLpiTxTimer = new cMessage("startLPITX", START_LPITX);
    wakeUpTimer = new cMessage("wakeUp", WAKE_UP);
    sendPresTimer = new cMessage("SendPres",SEND_PRES);
    plotTimer = new cMessage("plot", PLOT);
    wakedUpTimer = new cMessage("wake", WAKED_UP);
    quietTimer = new cMessage("quiet", QUIET);
    soaGoLpiTimer = new cMessage("SoaGoLPI",SOA_GOLPI);

    timerMsg = new cMessage("startPow",START_POW);
    scheduleAt(simTime(), timerMsg);

}

void ControlledNodeSrv::handleSelfMsg(cMessage *msg) {
	switch (msg->getKind()) {
		case START_POW:
			if (numHost == 1) {
				power = DefEEEPower;
				emit(rcvdPw_1Signal, power);
			}
			else if (numHost == 2) {
				power = DefNoEEEPower;
				emit(rcvdPw_2Signal, power);
			}

			power = DefEEEPower;
			emit(rcvdPwSignal, power); // save default power in LPI

			power = DefNoEEEPower;
			emit(rcvdPwNoEEESignal, power); // save default power in ACTIVE

			scheduleAt(simTime() + Tplt, plotTimer);

			break;
		case PLOT:
			scheduleAt(simTime() + Tplt, plotTimer);
			emit(rcvdPw_1Signal, zero);
			emit(rcvdPw_2Signal, zero);
			emit(rcvdPwSignal, zero);
			emit(rcvdPwNoEEESignal, zero);
			break;
		case WAKE_AFTER:
			wakeUp();
			break;
		case SOA_GOLPI: // Go LPI after SOA
			goLPI(); // Switch to LPI mode
			break;
		case START_LPI: // Start LPI mode
			if (numHost == 1) // local trasmitter in LPI
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
			break;
		case START_LPITX: // Start partial LPI mode
			if (numHost == 1) {
				power = L_TXRX_EEEPower;
				emit(rcvdPw_1Signal, power);
			}
			else if (numHost == 2) {
				power = LNoEEEPower;
				emit(rcvdPw_2Signal, power);
			}
			power = L_TXRX_EEEPower;
			emit(rcvdPwSignal, power);

			EV << "START LPI_TX";

			power = LNoEEEPower;
			emit(rcvdPwNoEEESignal, power);

			EV << "Scheduling quiet timer"<< endl;
			if(!quietTimer->isScheduled())
			    scheduleAt(simTime() + Tq, quietTimer);
			else opp_error("Already scheduled %s %g", this->getFullName(), quietTimer->getSendingTime().dbl());

			break;
		case QUIET: // Send LPI signals if the CN isn't waking up
			if (!wakedUp) {
				sendLPISignal(); // Send LPI signal on the link
				EV << "Sending periodic LPI Signal\n";
				scheduleAt(simTime() + Tq, quietTimer);
				EV << "Quiet\n";
			}
			else opp_error("Errore caso QUIET");
			break;
		case WAKE_UP: // Exit LPI mode
			wakedUp = true;
			EV << "Waking up\n";
			IEndTime = simTime();
			EV << "exit LPI " << IEndTime << "\n";
			scheduleAt(simTime() + Tw, wakedUpTimer);
			break;
		case WAKED_UP: // Change status in ACTIVE
			if (numHost == 1) {
				power = HEEEPower;
				emit(rcvdPw_1Signal, power);
			}
			else if (numHost == 2) {
				power = HNoEEEPower;
				emit(rcvdPw_2Signal, power);
			}
			EV << "STOP LPI";

			power = HEEEPower;
			emit(rcvdPwSignal, power);
			power = HNoEEEPower;
			emit(rcvdPwNoEEESignal, power);

			//perc = (IEndTime-IStartTime)/(nodesCycleCN[numHost-1]*TPowCycle);
			perc = (IEndTime - IStartTime).dbl();
			EV << "Tempo trascorso in LPI dall'Host " << numHost << "Tidle = "
					<< (IEndTime - IStartTime);
			break;
		case SEND_PRES: // Send PRes after TPreq_Pres_CN
			EV << "sending PRES\n";
			sendPacket(datapacket, srcAddr);
			if (numHost != 3 && numHost != 4 && numHost != 5 && !onSoa) {
				goLPI(); // Switch to LPI mode
			}
			break;
		default:
			delete msg;
			break;
	}

}

void ControlledNodeSrv::handleMessage(cMessage *msg){
    if (msg->isSelfMessage()) {
   	 handleSelfMsg(msg);
    }
    else {
        EPLframe *req = check_and_cast<EPLframe *>(msg); // EPL frame received

        if( req->getType() == PREQ  ) { // Preq received

      	  if(!onSoa){
      	  nextWake = simTime() + TPowCycle - Tw;
			  wakeAfterTimer = new cMessage("WakeAfter",WAKE_AFTER);
			  scheduleAt(nextWake, wakeAfterTimer);
      	  }

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
            scheduleAt(Tpres, sendPresTimer);
            k++;
        }
        else if (req->getType() == SOA)     // SoA received
        {
            goLPITX(); // fai partire il LPI parziale

            if(req->getHost() == numHost){ // SoA indexed to me, send ASnd and switch to LPI mode

                EV << "Received packet `" << msg->getName() << "'\n";
                packetsReceived++;
                emit(rcvdPkSignal, req);

                // ASnd frame Generation

                Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(req->removeControlInfo());
                srcAddr = ctrl->getSrc();
                char msgname[30];
                sprintf(msgname, "AsyncSendHost%d", numHost);

                delete ctrl;

                int k = 0;
                EV << "Generating packet `" << msgname << "'\n";
                datapacket = new EPLframe(msgname, IEEE802CTRL_DATA); // ASnd frame

                long len = reqLength->longValue();
                datapacket->setByteLength(len);
                datapacket->setType(ASYNC);
                sendPacket(datapacket, srcAddr);
                k++;
                scheduleAt(simTime() + TPropag, soaGoLpiTimer);

            }
            else // SoA not indexed to me, switch to LPI mode
            {
                if( numHost == 3 || numHost == 4 || numHost == 5 || onSoa)
                {
                    goLPI(); // Switch to LPI mode
                }
            }
            delete msg;
        }
        else if (req->getType() == STOP_IDLE &&  nextWake == 0 )  // wake up if stopIDLE received and it's our cycle
        {
            wakeUp();
            delete msg;
        }
        else{
            EV << "Received msg from lower layers, type " << req->getFullName() << endl;
            delete msg;
        }

    }
}

void ControlledNodeSrv::sendLPISignal()
{

        EV << "Generating LPI packet `LPISignal'\n";

        // LPI signal Generation

        datapacket = new EPLframe("LPISignal", IEEE802CTRL_DATA);
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

void ControlledNodeSrv::goLPI()  // Switch to LPI mode
{
    sendLPISignal();
    simtime_t TsWait = simTime() + Ts;

    scheduleAt(TsWait, startLpiTimer);
    EV << "Starting Low Power Idle\n";
    wakedUp = false;
}
void ControlledNodeSrv::goLPITX()  // Switch to LPI mode
{
    scheduleAt(simTime() + Ts, startLpiTxTimer);
    EV << "Starting Partial Low Power Idle\n";
}

void ControlledNodeSrv::wakeUp() // Switch to ACTIVE mode
{
    //cancelEvent(timerMsg);
    if(quietTimer->isScheduled())
        cancelEvent(quietTimer);
    scheduleAt(simTime(), wakeUpTimer);
}

void ControlledNodeSrv::sendPacket(cPacket *datapacket, const MACAddress& destAddr)
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

void ControlledNodeSrv::registerDSAP(int dsap)
{
    EV << getFullPath() << " registering DSAP " << dsap << "\n";

    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setDsap(dsap);
    cMessage *msg = new cMessage("register_DSAP", IEEE802CTRL_REGISTER_DSAP);
    msg->setControlInfo(etherctrl);

    send(msg, "out");
}

void ControlledNodeSrv::finish()
{
    cancelAndDelete(timerMsg);
    cancelAndDelete(sendPresTimer);
    cancelAndDelete(startLpiTxTimer);
    cancelAndDelete(wakeUpTimer);
    cancelAndDelete(startLpiTimer);
    cancelAndDelete(quietTimer);
    cancelAndDelete(plotTimer);
    cancelAndDelete(soaGoLpiTimer);
    cancelAndDelete(wakedUpTimer);
}
