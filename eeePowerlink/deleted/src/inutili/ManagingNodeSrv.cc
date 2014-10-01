
#include <stdio.h>
#include <string.h>

#include "ManagingNodeSrv.h"

#include "EtherApp_m.h"
#include "Ieee802Ctrl_m.h"



Define_Module(ManagingNodeSrv);

simsignal_t ManagingNodeSrv::sentPkSignal = SIMSIGNAL_NULL;
simsignal_t ManagingNodeSrv::rcvdPkSignal = SIMSIGNAL_NULL;

void ManagingNodeSrv::initialize()
{
    localSAP = ETHERAPP_SRV_SAP;
    remoteSAP = ETHERAPP_CLI_SAP;

    // statistics
    packetsSent = packetsReceived = 0;
    sentPkSignal = registerSignal("sentPk");
    rcvdPkSignal = registerSignal("rcvdPk");

    numHost = par("numHost");

    sprintf(SoAString, "SoA%d", numHost);

    EV << "My number is `" << SoAString << "'\n";

    WATCH(packetsSent);
    WATCH(packetsReceived);

    bool registerSAP = par("registerSAP");
    if (registerSAP)
        registerDSAP(localSAP);
}

void ManagingNodeSrv::handleMessage(cMessage *msg)
{
    EV << "Received packet `" << msg->getName() << "'\n";
    EtherAppReq *req = check_and_cast<EtherAppReq *>(msg);
    packetsReceived++;
    emit(rcvdPkSignal, req);

    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(req->removeControlInfo());
    MACAddress srcAddr = ctrl->getSrc();
    long requestId = req->getRequestId();
    long replyBytes = req->getResponseBytes();
    char msgname[30];
    strcpy(msgname, msg->getName());

    delete msg;
    delete ctrl;

    // send back packets asked by EtherAppCli side
    int k = 0;
    strcat(msgname, "-resp-");
    char *s = msgname + strlen(msgname);

    while (replyBytes > 0)
    {
        int l = replyBytes > MAX_REPLY_CHUNK_SIZE ? MAX_REPLY_CHUNK_SIZE : replyBytes;
        replyBytes -= l;

        sprintf(s, "%d", k);

        EV << "Generating packet `" << msgname << "'\n";

        EtherAppResp *datapacket = new EtherAppResp(msgname, IEEE802CTRL_DATA);
        datapacket->setRequestId(requestId);
        datapacket->setByteLength(l);
        sendPacket(datapacket, srcAddr);

        k++;
    }
}
void ManagingNodeSrv::sendPacket(cPacket *datapacket, const MACAddress& destAddr)
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

void ManagingNodeSrv::registerDSAP(int dsap)
{
    EV << getFullPath() << " registering DSAP " << dsap << "\n";

    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setDsap(dsap);
    cMessage *msg = new cMessage("register_DSAP", IEEE802CTRL_REGISTER_DSAP);
    msg->setControlInfo(etherctrl);

    send(msg, "out");
}

void ManagingNodeSrv::finish()
{
}

