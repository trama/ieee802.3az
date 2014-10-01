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

#include "EEE_EtherTrafGen.h"

#include "Ieee802Ctrl_m.h"
#include "NodeOperations.h"
#include "ModuleAccess.h"

Define_Module(EEE_EtherTrafGen);

simsignal_t EEE_EtherTrafGen::sentPkSignal = SIMSIGNAL_NULL;
simsignal_t EEE_EtherTrafGen::rcvdPkSignal = SIMSIGNAL_NULL;

EEE_EtherTrafGen::EEE_EtherTrafGen(){
    sendInterval = NULL;
    numPacketsPerBurst = NULL;
    packetLength = NULL;
    timerMsg = NULL;
    nodeStatus = NULL;
}

EEE_EtherTrafGen::~EEE_EtherTrafGen(){
    cancelAndDelete(timerMsg);
}

void EEE_EtherTrafGen::initialize(int stage){
    // we can only initialize in the 2nd stage (stage==1), because
    // assignment of "auto" MAC addresses takes place in stage 0
    if (stage == 1)
    {
        sendInterval = &par("sendInterval");
        numPacketsPerBurst = &par("numPacketsPerBurst");
        packetLength = &par("packetLength");
        etherType = par("etherType");

        seqNum = 0;
        WATCH(seqNum);

        // statistics
        packetsSent = packetsReceived = 0;
        sentPkSignal = registerSignal("sentPk");
        rcvdPkSignal = registerSignal("rcvdPk");
        WATCH(packetsSent);
        WATCH(packetsReceived);

        startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            error("Invalid startTime/stopTime parameters");

        if (isGenerator())
            timerMsg = new cMessage("generateNextPacket");

        nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));

        if (isNodeUp() && isGenerator())
            scheduleNextPacket(-1);
    }
}

void EEE_EtherTrafGen::handleMessage(cMessage *msg){
    if (!isNodeUp())
        throw cRuntimeError("Application is not running");
    if (msg->isSelfMessage())
    {
        if (msg->getKind() == START)
        {
            destMACAddress = resolveDestMACAddress();
            // if no dest address given, nothing to do
            if (destMACAddress.isUnspecified())
                return;
        }
        sendBurstPackets();
        scheduleNextPacket(simTime());
    }
    else
        receivePacket(check_and_cast<cPacket*>(msg));
}

bool EEE_EtherTrafGen::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_APPLICATION_LAYER && isGenerator())
            scheduleNextPacket(-1);
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER)
            cancelNextPacket();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (stage == NodeCrashOperation::STAGE_CRASH)
            cancelNextPacket();
    }
    else throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

bool EEE_EtherTrafGen::isNodeUp()
{
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

bool EEE_EtherTrafGen::isGenerator()
{
    return par("destAddress").stringValue()[0];
}

void EEE_EtherTrafGen::scheduleNextPacket(simtime_t previous)
{
    simtime_t next;
    if (previous == -1)
    {
        next = simTime() <= startTime ? startTime : simTime();
        timerMsg->setKind(START);
    }
    else
    {
        next = previous + sendInterval->doubleValue();
        timerMsg->setKind(NEXT);
    }
    if (stopTime < SIMTIME_ZERO || next < stopTime)
        scheduleAt(next, timerMsg);
}

void EEE_EtherTrafGen::cancelNextPacket()
{
    cancelEvent(timerMsg);
}

MACAddress EEE_EtherTrafGen::resolveDestMACAddress()
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

void EEE_EtherTrafGen::sendBurstPackets()
{
    int n = numPacketsPerBurst->longValue();
    for (int i = 0; i < n; i++)
    {
        seqNum++;

        char msgname[40];
        sprintf(msgname, "pk-%d-%ld", getId(), seqNum);
        EV << "Generating packet `" << msgname << "'\n";

        cPacket *datapacket = new cPacket(msgname, IEEE802CTRL_DATA);

        long len = packetLength->longValue();
        datapacket->setByteLength(len);

        Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
        etherctrl->setEtherType(etherType);
        etherctrl->setDest(destMACAddress);
        datapacket->setControlInfo(etherctrl);

        emit(sentPkSignal, datapacket);
        send(datapacket, "out");
        packetsSent++;
    }
}

void EEE_EtherTrafGen::receivePacket(cPacket *msg)
{
    EV << "Received packet `" << msg->getName() << "'\n";

    packetsReceived++;
    emit(rcvdPkSignal, msg);
    delete msg;
}

void EEE_EtherTrafGen::finish()
{
    cancelAndDelete(timerMsg);
    timerMsg = NULL;
}

