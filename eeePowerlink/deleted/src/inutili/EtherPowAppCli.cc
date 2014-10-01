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

#include "EtherPowAppCli.h"

#include "EtherApp_m.h"
#include "Ieee802Ctrl_m.h"


Define_Module(EtherPowAppCli);

simsignal_t EtherPowAppCli::sentPkSignal = SIMSIGNAL_NULL;
simsignal_t EtherPowAppCli::rcvdPkSignal = SIMSIGNAL_NULL;
bool wakeUp = true;

EtherPowAppCli::EtherPowAppCli()
{
    timerMsg = NULL;
}
EtherPowAppCli::~EtherPowAppCli()
{
    cancelAndDelete(timerMsg);
}

void EtherPowAppCli::initialize(int stage)
{
    // we can only initialize in the 2nd stage (stage==1), because
    // assignment of "auto" MAC addresses takes place in stage 0
    if (stage == 1)
    {
        reqLength = &par("reqLength");
        respLength = &par("respLength");
        sendInterval = &par("sendInterval");

        localSAP = ETHERAPP_CLI_SAP;
        remoteSAP = ETHERAPP_CLI_SAP;

        seqNum = 0;
        WATCH(seqNum);

        // statistics
        packetsSent = packetsReceived = 0;
        sentPkSignal = registerSignal("sentPk");
        rcvdPkSignal = registerSignal("rcvdPk");
        WATCH(packetsSent);
        WATCH(packetsReceived);

        destMACAddress = resolveDestMACAddress();

        // if no dest address given, nothing to do
        //if (destMACAddress.isUnspecified())
        //   return;

        bool registerSAP = par("registerSAP");
        if (registerSAP)
            registerDSAP(localSAP);

        simtime_t startTime = par("startTime");
        stopTime = par("stopTime");
        Ts = par("Ts");
        Tq = par("Tq");
        Tw = par("Tw");
        TwakeUp = par("TwakeUp");
        if (stopTime != 0 && stopTime <= startTime)
            error("Invalid startTime/stopTime parameters");

        timerMsg = new cMessage("generateNextPacket");
        scheduleAt(startTime, timerMsg);

    }
}

MACAddress EtherPowAppCli::resolveDestMACAddress()
{
    MACAddress destMACAddress;
    const char *destAddress = par("destAddress");
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

void EtherPowAppCli::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if(strcmp(msg->getName(), "generateNextPacket")==0){
            sendPacket();
            simtime_t TsWait = simTime() + Ts;
            timerMsg = new cMessage("startLPI");
            scheduleAt(TsWait, timerMsg);
            EV << "Starting Low Power Idle\n";
            wakeUp = false;
        }
        else if(strcmp(msg->getName(), "startLPI")==0){
            simtime_t TqWait = simTime() + Tq;
            timerMsg = new cMessage("quiet");
            scheduleAt(TqWait, timerMsg);
            simtime_t Twake = simTime() + TwakeUp;             // wake up in 1s
            timerMsg = new cMessage("wakeUp");
            scheduleAt(Twake, timerMsg);
            EV << "Quiet\n";
        }
        else if( ( strcmp(msg->getName(), "quiet") == 0 ) && ( !wakeUp )){
            sendLPISignal();
            EV << "Sending LTI Signal\n";
            simtime_t TqWait = simTime() + Tq;
            timerMsg = new cMessage("quiet");
            scheduleAt(TqWait, timerMsg);
            EV << "Quiet\n";
        }
        else if(strcmp(msg->getName(), "wakeUp")==0){
            wakeUp = true;
            EV << "Waking up\n";
            simtime_t TwWait = simTime() + Tw;
            timerMsg = new cMessage("wake");
            scheduleAt(TwWait, timerMsg);
        }
        else if(strcmp(msg->getName(), "wake")==0){
            EV << "Active\n";
            sendPacket();
        }
    }
    /*else
    {
        receivePacket(check_and_cast<cPacket*>(msg));
    }*/
}

void EtherPowAppCli::registerDSAP(int dsap)
{
    EV << getFullPath() << " registering DSAP " << dsap << "\n";

    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setDsap(dsap);
    cMessage *msg = new cMessage("register_DSAP", IEEE802CTRL_REGISTER_DSAP);
    msg->setControlInfo(etherctrl);
    send(msg, "out");
}

void EtherPowAppCli::sendPacket()
{
    char msgname[30];
    sprintf(msgname, "req-%d-%ld", getId(), seqNum);
    EV << "Generating packet `" << msgname << "'\n";

    EtherAppReq *datapacket = new EtherAppReq(msgname, IEEE802CTRL_DATA);

    datapacket->setRequestId(seqNum);

    long len = reqLength->longValue();
    datapacket->setByteLength(len);

    long respLen = respLength->longValue();
    datapacket->setResponseBytes(respLen);

    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setSsap(localSAP);
    etherctrl->setDsap(remoteSAP);
    etherctrl->setDest(destMACAddress);
    datapacket->setControlInfo(etherctrl);

    emit(sentPkSignal, datapacket);
    send(datapacket, "out");
    packetsSent++;
}

void EtherPowAppCli::sendLPISignal()
{

        char msgname[30];
        sprintf(msgname, "LPISignal");
        EV << "Generating LPI packet `" << msgname << "'\n";

        EtherAppReq *datapacket = new EtherAppReq(msgname, IEEE802CTRL_DATA);

        datapacket->setRequestId(seqNum);

        long len = reqLength->longValue();
        datapacket->setByteLength(len);

        long respLen = respLength->longValue();
        datapacket->setResponseBytes(respLen);

        Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
        etherctrl->setSsap(localSAP);
        etherctrl->setDsap(remoteSAP);
        datapacket->setControlInfo(etherctrl);

        emit(sentPkSignal, datapacket);
        send(datapacket, "out");
        packetsSent++;

}


void EtherPowAppCli::receivePacket(cPacket *msg)
{
    EV << "Received LPI packet `" << msg->getName() << "'\n";

    packetsReceived++;
    emit(rcvdPkSignal, msg);
    delete msg;
}

void EtherPowAppCli::finish()
{
    cancelAndDelete(timerMsg);
    timerMsg = NULL;
}

