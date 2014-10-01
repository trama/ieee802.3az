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
#include <math.h>
#include <omnetpp.h>
#include <MACAddress.h>
#include "powerListener.h"


#include "ManagingNodeCli_noEEE.h"

#include "EtherApp_m.h"
#include "EPLframe_m.h"
//#include "EPLframeKind.h"

#include "Ieee802Ctrl_m.h"

Define_Module(ManagingNodeCli_noEEE);

simsignal_t ManagingNodeCli_noEEE::sentPkSignal = SIMSIGNAL_NULL;
simsignal_t ManagingNodeCli_noEEE::rcvdPkSignal = SIMSIGNAL_NULL;
simsignal_t ManagingNodeCli_noEEE::rcvdPwSignal = SIMSIGNAL_NULL;
simsignal_t ManagingNodeCli_noEEE::rcvdPwNoEEESignal = SIMSIGNAL_NULL;

enum MNNumbers {
    //N = 14
    //N = 10
    //N = 13
    N = 3
};

int mp1 = 1;
int h1 = 1; // index of the current node
int currentHost1; // Host for AsyncSend
bool persoPres1 = false; // Loss of PRes

ManagingNodeCli_noEEE::ManagingNodeCli_noEEE()
{
    timerMsg = NULL;
}
ManagingNodeCli_noEEE::~ManagingNodeCli_noEEE()
{
    cancelAndDelete(timerMsg);
}

void ManagingNodeCli_noEEE::initialize(int stage)
{
    // we can only initialize in the 2nd stage (stage==1), because
    // assignment of "auto" MAC addresses takes place in stage 0
    if (stage == 1)
    {
        reqLength = &par("reqLength");
        respLength = &par("respLength");
        sendInterval = &par("sendInterval");
        cycleCount = 0;

        // Powerlink cycle parameters
        TPowCycle = par("TPowCycle"); // EPL cycle time
        TPreq_Pres_CN = par("TPreq_Pres_CN"); // time between Preq and relative Pres
        TPropag = par("TPropag"); // propagation time
        Tw = par("Tw"); // waking time
        TwaitPres = TPreq_Pres_CN + 2*TPropag; // timeout for Pres
        TCN = Tw + TPropag; // Wake up waiting time: TwakeUp + Tpropagation


        timerEnd = false;
        // Nodes
        EV << "nodes:" << N <<"\n";
        for(int i=1; i<=N; i++){
            sprintf (nodes[i], "Host%d", i);
            EV << nodes[i] << "\n";
        }


        localSAP = ETHERAPP_CLI_SAP;
        remoteSAP = ETHERAPP_SRV_SAP;

        seqNum = 0;
        WATCH(seqNum);

        // statistics
        packetsSent = packetsReceived = 0;
        sentPkSignal = registerSignal("sentPk");
        rcvdPkSignal = registerSignal("rcvdPk");
        rcvdPwSignal = registerSignal("rcvdPw");
        rcvdPwNoEEESignal = registerSignal("rcvdPwNoEEE");


        powerListener *listener = new powerListener(true);
        simulation.getSystemModule()->subscribe(rcvdPwSignal, listener);
        powerListener *listenerNoEEE = new powerListener(false);
        simulation.getSystemModule()->subscribe(rcvdPwNoEEESignal, listenerNoEEE);

        WATCH(packetsSent);
        WATCH(packetsReceived);

        bool registerSAP = par("registerSAP");
        if (registerSAP)
            registerDSAP(localSAP);

        simtime_t startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime != 0 && stopTime <= startTime)
            error("Invalid startTime/stopTime parameters");

        timerMsg = new cMessage("startCycle",WAKE_UP); // Start cycle waking up the CNs
        scheduleAt(startTime, timerMsg);
    }
}

MACAddress ManagingNodeCli_noEEE::resolveDestMACAddress(char *destAddress)
{
    MACAddress destMACAddress;

    if (destAddress[0])
    {
        // try as mac address first, then as a module
        if (!destMACAddress.tryParse(destAddress))
        {
            cModule *destStation = simulation.getModuleByPath(destAddress);
            if (!destStation)
                error("cannot resolve MAC address '%s': not a 12-hex-digit MAC address or a valid module path name", destAddress);

            cModule *destMAC = destStation->getSubmodule("mac");
            if (!destMAC)
                error("module '%s' has no 'mac' submodule", destAddress);

            destMACAddress.setAddress(destMAC->par("address"));
        }
    }
    return destMACAddress;
}

void ManagingNodeCli_noEEE::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if(msg->getKind() == WAKE_UP)         // Wake Up the CNs
        {
            cycleCount++; // start new cycle
            TimeEPL = simTime();
            // Stop the IDLE fase
            //sendPacket(STOP_IDLE,h1);
            timerMsg = new cMessage("StopIDLE",WAKE_UP);
            EV << "Fine fase di IDLE\n";
            d = simTime() + TPowCycle;
            scheduleAt(d, timerMsg);
            // Start of the EPL cycle
            d = simTime() + TCN;
            timerMsg = new cMessage("startPowerLinkCycle",SEND_SOC);
            scheduleAt(d, timerMsg);
        }
        else if(msg->getKind() == SEND_SOC)        // SoC transmission
        {
            TimeEPL = simTime() - TimeEPL;
            if(cycleCount>1)
            {
                printf("Tempo necessario a risvegliare i nodi: %f us\n", TimeEPL.dbl()*1000000);
            }
            sendPacket(SOC,h1); // Inizio del Ciclo Powerlink
            d = simTime() + TPropag; // Wait SoC propagation time before starting CNs polling
            timerMsg = new cMessage("Preq",POLLING);
            scheduleAt( d, timerMsg);

            TimeEPL = simTime();

        }
        else if(msg->getKind() == POLLING)        // Preq polling
        {

            if( h1 > 1 )
            {
                TimePOLL = simTime()-TimePOLL;
                printf("Tempo necessario ad effettuare il poll con l'Host%d: %f us\n",h1-1, TimePOLL.dbl()*1000000);
            }

            TimePOLL = simTime();

            if(persoPres1) // Loss of PRes
            {
                EV << "Scartato pacchetto Pres" << currentHost1 << " \n";
            }

            persoPres1 = true;
            currentHost1 = h1; // current Host
            h1++; // next Host



            EV << "destinazione" << nodes[currentHost1] << "\n";
            sendPacket(PREQ, currentHost1);
            EV << "Preq trasmesso, avvio il timeout \n";

            if(h1 == N+1) // Wait the timeout and send the SoA if polling is completed
            {
                d = simTime() + TwaitPres;
                timerMsg = new cMessage("SoA",SEND_SOA);
                scheduleAt(d, timerMsg);
            }
            else // Wait the timeout and send the next PReq
            {
                d = simTime() + TwaitPres;
                timerMsg = new cMessage("Preq",POLLING);
                scheduleAt(d, timerMsg);
            }
        }
        else if(msg->getKind() == SEND_SOA)         // SoA trasmission
        {
            TimePOLL = simTime()-TimePOLL;
            printf("Tempo necessario ad effettuare il poll con l'Host%d: %f us\n",h1-1, TimePOLL.dbl()*1000000);
            TimeEPL = simTime() - TimeEPL;
            printf("Durata fase isocrona: %f us\n", TimeEPL.dbl()*1000000);
            //SoAHost = (rand() % N) + 1; // Random Host between 1 and n
            SoAHost = (rand() % 3) + 3; // Random Host between 1 and 3
            TimeAsync = simTime();
            sendPacket(SOA, SoAHost);
            h1 = 1; // First Host for the next EPL cycle
        }
    }
    else
    {
        EPLframe *req = check_and_cast<EPLframe *>(msg); // EPL frame received
        EV<< "tipo " << req->getType() << "host" << req->getHost()<< "\n";
        if( req->getType() == PRES &&  req->getHost() == currentHost1) // Pres received
        {
            persoPres1 = false;
            cancelEvent(timerMsg); // reset timeout
            EV<< "Ricevuto PRes_Host" << currentHost1 << " azzero il Timer \n";
            if(h1 == N+1) // Send the SoA if polling is completed
            {
                EV<< "Il polling è terminato, procedo con la fase Asincrona \n";
                timerMsg = new cMessage("SoA",SEND_SOA);
                scheduleAt(simTime(), timerMsg);
            }
            else // Send the next PReq
            {
                EV<< "Il polling non è terminato, procedo con il prossimo Host \n";
                timerMsg = new cMessage("Preq",POLLING);
                scheduleAt(simTime(), timerMsg);
            }

        }
        else if(req->getType() == ASYNC)        // Preq polling
        {
            TimeAsync = simTime() - TimeAsync;
            printf("Durata fase asincrona: %f us\n", TimeAsync.dbl()*1000000);
            printf("Durata fase idle: %f us\n", (TPowCycle - TimeAsync - TimeEPL).dbl()*1000000);
        }
    }

}

void ManagingNodeCli_noEEE::registerDSAP(int dsap)
{
    EV << getFullPath() << " registering DSAP " << dsap << "\n";

    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setDsap(dsap);
    cMessage *msg = new cMessage("register_DSAP", IEEE802CTRL_REGISTER_DSAP);
    msg->setControlInfo(etherctrl);
    send(msg, "out");
}


void ManagingNodeCli_noEEE::sendPacket(int type, int dest)
{
    seqNum++;
    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    char msgname[30];
    switch ( type ) {
    case SOC: // SoC
        sprintf(msgname, "SoC");
        etherctrl->setDest(MACAddress::BROADCAST_ADDRESS);
        break;
    case PREQ: // Preq
        sprintf(msgname, "PReq_Host%d", dest);
        destMACAddress = resolveDestMACAddress(nodes[dest]);
        etherctrl->setDest(destMACAddress);
        break;
    case SOA: // SoA
        sprintf(msgname, "SoA%d", dest);
        etherctrl->setDest(MACAddress::BROADCAST_ADDRESS);
        break;
    case STOP_IDLE: // StopIdle
        sprintf(msgname, "StopIDLE");
        etherctrl->setDest(MACAddress::BROADCAST_ADDRESS);
        break;
    }


    EPLframe *datapacket = new EPLframe(msgname, IEEE802CTRL_DATA);

    long len = reqLength->longValue();
    datapacket->setByteLength(len);
    datapacket->setType(type);
    datapacket->setMp(cycleCount);
    datapacket->setHost(dest);
    etherctrl->setSsap(localSAP);
    etherctrl->setDsap(remoteSAP);
    datapacket->setControlInfo(etherctrl);

    EV << "Generating packet `" << msgname << "' with length " << len<<"\n";
    emit(sentPkSignal, datapacket);
    send(datapacket, "out");
    packetsSent++;

}


void ManagingNodeCli_noEEE::receivePacket(cPacket *msg)
{
    EV << "Received packet `" << msg->getName() << "'\n";

    packetsReceived++;
    emit(rcvdPkSignal, msg);
    delete msg;
}

void ManagingNodeCli_noEEE::finish()
{
    cancelAndDelete(timerMsg);
    timerMsg = NULL;
}

