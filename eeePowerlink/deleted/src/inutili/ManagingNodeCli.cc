
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <omnetpp.h>
#include <MACAddress.h>
#include "powerListener.h"


#include "ManagingNodeCli.h"

#include "EtherApp_m.h"
#include "EPLframe_m.h"
//#include "EPLframeKind.h"

#include "Ieee802Ctrl_m.h"

Define_Module(ManagingNodeCli);

void ManagingNodeCli::initialize(int stage)
{
    // we can only initialize in the 2nd stage (stage==1), because
    // assignment of "auto" MAC addresses takes place in stage 0
    if (stage == 1)
    {
        reqLength = &par("reqLength");
        respLength = &par("respLength");
        sendInterval = &par("sendInterval");
        cycleCount = 0;

        mp = 1;
        h = 1; // index of the current node
        persoPres = false; // Loss of PRes

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

        socTimer = new cMessage("startPowerLinkCycle", SEND_SOC);
        wakeUpTimer = new cMessage("StopIDLE", WAKE_UP);
        preqTimer = new cMessage("Preq", POLLING);
        preqTimeoutTimer = new cMessage("Preq timeout", PREQ_TIMEOUT);
        soaTimer = new cMessage("SoA", SEND_SOA);

        timerMsg = new cMessage("startCycle",WAKE_UP); // Start cycle waking up the CNs
        scheduleAt(startTime, timerMsg);
    }
}

void ManagingNodeCli::handleSelfMsg(cMessage *msg) {
    switch (msg->getKind()) {
    case WAKE_UP: // Wake Up the CNs
        cycleCount++; // start new cycle
        TimeEPL = simTime();
        // Stop the IDLE fase
        sendPacket(STOP_IDLE, h);
        EV << "Fine fase di IDLE\n";
        scheduleAt(simTime() + TPowCycle, wakeUpTimer);

        // Start of the EPL cycle
        scheduleAt(simTime() + TCN, socTimer);

        break;
    case SEND_SOC: // SoC transmission
        TimeEPL = simTime() - TimeEPL;
        if (cycleCount > 1) {
            EV << "Tempo necessario a risvegliare i nodi: "
                    << TimeEPL.dbl() * 1000000 << "us\n";
        }
        sendPacket(SOC, h); // Inizio del Ciclo Powerlink

        scheduleAt(simTime() + TPropag, preqTimer); // Wait SoC propagation time before starting CNs polling

        TimeEPL = simTime();
        break;
    case POLLING: // Preq polling
        if (h > 1) {
            TimePOLL = simTime() - TimePOLL;
            EV << "Tempo necessario ad effettuare il poll con l'Host " << h - 1
                    << " : " << TimePOLL.dbl() * 1000000 << "us\n";
        }

        TimePOLL = simTime();

        if (persoPres) // Loss of PRes
        {
            EV << "Scartato pacchetto Pres" << currentHost << " \n";
        }

        persoPres = true;
        currentHost = h; // current Host
        h++; // next Host

        EV << "destinazione" << nodes[currentHost] << "\n";
        sendPacket(PREQ, currentHost);
        EV << "Preq trasmesso, avvio il timeout \n";

        scheduleAt(simTime() + TwaitPres, preqTimeoutTimer);
        break;
    case PREQ_TIMEOUT:
        if (h == N + 1) // Wait the timeout and send the SoA if polling is completed
                scheduleAt(simTime(), soaTimer);
        else // Wait the timeout and send the next PReq
                scheduleAt(simTime(), preqTimer);
        break;
    case SEND_SOA: // SoA trasmission
        TimePOLL = simTime() - TimePOLL;
        EV << "Tempo necessario ad effettuare il poll con l'Host " << h - 1
                << " : " << TimePOLL.dbl() * 1000000 << "us\n";
        //printf("Tempo necessario ad effettuare il poll con l'Host%d: %f us\n",h-1, TimePOLL.dbl()*1000000);
        TimeEPL = simTime() - TimeEPL;
        EV << "Durata fase isocrona: " << TimeEPL.dbl() * 1000000 << "us\n";
        //printf("Durata fase isocrona: %f us\n", TimeEPL.dbl()*1000000);
        SoAHost = (rand() % 3) + 3; // Random Host between 1 and 3
        TimeAsync = simTime();
        sendPacket(SOA, SoAHost);
        h = 1; // First Host for the next EPL cycle
        break;
    default:
        opp_error("Non so");
        break;
    }
}

void ManagingNodeCli::handleMessage(cMessage *msg){
    if (msg->isSelfMessage()) {
        handleSelfMsg(msg);
    }
    else {

        EPLframe *req = check_and_cast<EPLframe *>(msg); // EPL frame received
        EV<< "tipo " << req->getType() << "host" << req->getHost()<< "\n";
        if( req->getType() == PRES &&  req->getHost() == currentHost) // Pres received
        {
            persoPres = false;

            EV<< "Ricevuto PRes_Host" << currentHost << " azzero il Timer \n";
            if(h == N+1) // Send the SoA if polling is completed
            {
                cancelEvent(preqTimeoutTimer); // reset timeout
                EV<< "Il polling è terminato, procedo con la fase Asincrona \n";
                scheduleAt(simTime(), soaTimer);
            }
            else // Send the next PReq
            {
                cancelEvent(preqTimeoutTimer); // reset timeout
                EV<< "Il polling non è terminato, procedo con il prossimo Host \n";
                scheduleAt(simTime(), preqTimer);
            }

        }
        else if(req->getType() == ASYNC)        // Preq polling
        {
            TimeAsync = simTime() - TimeAsync;
            EV << "Durata fase asincrona: " << TimeAsync.dbl()*1000000 << "us\n";
            EV << "Durata fase idle: " << (TPowCycle - TimeAsync - TimeEPL).dbl()*1000000 << "us\n";
        }
      delete msg;
    }

}

void ManagingNodeCli::registerDSAP(int dsap)
{
    EV << getFullPath() << " registering DSAP " << dsap << "\n";

    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setDsap(dsap);
    cMessage *msg = new cMessage("register_DSAP", IEEE802CTRL_REGISTER_DSAP);
    msg->setControlInfo(etherctrl);
    send(msg, "out");
}


void ManagingNodeCli::sendPacket(int type, int dest)
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


void ManagingNodeCli::receivePacket(cPacket *msg)
{
    EV << "Received packet `" << msg->getName() << "'\n";

    packetsReceived++;
    emit(rcvdPkSignal, msg);
    delete msg;
}

void ManagingNodeCli::finish()
{
    cancelAndDelete(timerMsg);
    cancelAndDelete(preqTimer);
    cancelAndDelete(socTimer);
    cancelAndDelete(wakeUpTimer);
    cancelAndDelete(soaTimer);
    cancelAndDelete(preqTimeoutTimer);
}


MACAddress ManagingNodeCli::resolveDestMACAddress(char *destAddress)
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
