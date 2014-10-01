
#include "EPL_ControlledNode_intel.h"

#include "EtherApp_m.h"
#include "Ieee802Ctrl_m.h"
#include "EPLframe_m.h"
#include "powerListener.h"
#include <cassert>
#include "EtherMACBase.h"

Define_Module(EPL_ControlledNode_intel);

void EPL_ControlledNode_intel::initialize() {

    debug = true;
    hostAwake = false;
    onlyRx = false; //flag x capire se sono in LPI parziale attivo
    partialLPI = par("partialLPI").boolValue();

    reqLength = par("reqLength").longValue();
    numHost = par("numHost").longValue();

    // Subscribing to LPI_indication signal to track LPI state change
    EVT << "Submodule " << this->getParentModule()->getSubmodule("lpi_client")->getFullPath() << endl;
    this->getParentModule()->getSubmodule("lpi_client")->subscribe("LPI_indication", this);

    seqNum = 0;
    WATCH(seqNum);

    // statistics
    packetsSent = packetsReceived = 0;
    sentPkSignal = registerSignal("sentPk");
    rcvdPkSignal = registerSignal("rcvdPk");

    TPreq_Pres_CN = par("TPreq_Pres_CN");

    subNetIndex = par("subNetIndex");
    autoSetMACaddress = par("autoSetMACaddress");

    presTimer = new cMessage("SendPres", SEND_PRES);

    sprintf(SoAString, "SoA%d", numHost);
    EVT<< "My number is `" << numHost << "'\n";

    WATCH(packetsSent);
    WATCH(packetsReceived);

    //bool registerSAP = par("registerSAP");

    //if ( registerSAP ) registerDSAP(localSAP);

    if(subNetIndex >= 0)
        setMACAddress(subNetIndex);


}

void EPL_ControlledNode_intel::receiveSignal(cComponent* source, simsignal_t signalID,
        const char* s) {

    EVT << "Received signal from LPI client " << source->getFullPath() <<
            ", signal --> " << s << endl;

    if( ! this->getParentModule()->getSubmodule( source->getName() ) ) // controllo che il segnale arrivi dal "mio" lpi_client
        throw cRuntimeError("Not a submodule of this compound module (%s)!",this->getFullName());


    if(strcmp(s,"AWAKE") == 0){
        EVT << "Received awake signal. Node is now awake\n";
        hostAwake = true;
        EV << "Host can only receive? " << onlyRx << " now switching" << endl;
        onlyRx = false;
    }
    else if(strcmp(s,"partialAWAKE") == 0){
            EVT << "Now in partial LPI\n";
            hostAwake = true;
            onlyRx = true;
            if(!partialLPI)
                throw cRuntimeError("Module params not consistent!");
    }
    else if(strcmp(s,"LPI") == 0){
        EVT << "Now in LPI\n";
        hostAwake = false;
    }

}

void EPL_ControlledNode_intel::handleSelfMsg( cMessage *msg ) {

    switch ( msg->getKind() ) {
        case SEND_PRES: // Send PRes after TPreq_Pres_CN
            EVT<< "Sending PRES\n";
            sendPacket(datapacket, srcAddr);
            break;
        }
}

void EPL_ControlledNode_intel::handleMessage( cMessage *msg ) {
    if ( msg->isSelfMessage() ) {
        handleSelfMsg(msg);
    }
    else {
        EPLframe *req = check_and_cast<EPLframe *>(msg); // EPL frame received
        Ieee802Ctrl *ctrl;

        switch ( req->getType() ) {
            case SOC:
                EVT << "Arrived SoC" << endl;
                delete msg;
                break;

            case PREQ:// Preq received
                EVT << "Received Preq from MN\n";
                packetsReceived++;
                emit(rcvdPkSignal, req);

                // PRes frame Generation
                ctrl = check_and_cast<Ieee802Ctrl *>(req->removeControlInfo());
                srcAddr = ctrl->getSrc();
                char msgname[30];
                strcpy(msgname, msg->getName());
                msgname[3] = 's';
                delete msg;
                delete ctrl;

                EVT << "Generating packet Pres from Host "<< numHost <<endl;
                datapacket = new EPLframe(msgname, IEEE802CTRL_DATA); // Pres frame

                datapacket->setByteLength(reqLength);
                datapacket->setType(PRES);
                datapacket->setHost(numHost);
                scheduleAt(simTime() + TPreq_Pres_CN, presTimer);
                break;

            case SOA: // SoA received

                if ( req->getHost() == numHost ) { // SoA indexed to me, send ASnd and switch to LPI mode
                    //if(!partialLPI){
                    //    throw cRuntimeError("Cannot have soa messages and partial LPI for now!");

                    EVT<< "Received packet `" << msg->getName() << "'\n";
                    packetsReceived++;
                    emit(rcvdPkSignal, req);

                    // ASnd frame Generation
                    ctrl = check_and_cast<Ieee802Ctrl *>(req->removeControlInfo());
                    srcAddr = ctrl->getSrc();
                    char msgname[30];
                    sprintf(msgname, "AsyncSendHost%d", numHost);
                    delete msg;
                    delete ctrl;

                    EVT << "Generating packet `" << msgname << "'\n";
                    datapacket = new EPLframe(msgname, IEEE802CTRL_DATA);// ASnd frame
                    datapacket->setByteLength(reqLength);
                    datapacket->setType(ASYNC);
                    sendPacket(datapacket, srcAddr);
                }
                else  // SoA not indexed to me, switch to LPI mode, provided I was not already there
                    delete msg;

                break;

            default:
                EVT << "Received msg from lower layers, type " << req->getFullName() << endl;
                delete msg;
                //opp_error("Error!!!");
                break;
            }
        }
    }

void EPL_ControlledNode_intel::sendPacket( EPLframe *datapacket, const MACAddress& destAddr ) {
    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setDest(MACAddress::BROADCAST_ADDRESS);
    if ( datapacket->getType() == ASYNC )
        etherctrl->setDest(destAddr);
    datapacket->setControlInfo(etherctrl);
    emit(sentPkSignal, datapacket);
    send(datapacket, "out");
    packetsSent++;
}

void EPL_ControlledNode_intel::finish() {
    cancelAndDelete(presTimer);
}

MACAddress EPL_ControlledNode_intel::setMACAddress(int subNetIndex) {

    if (subNetIndex < 0)
        throw cRuntimeError("Error in setting MAC addr.");

    cModule *myStation = this->getParentModule();
    if (!myStation)
        error("Where is my parent host?");

    cModule *myMAC = myStation->getSubmodule("mac");

    if (!myMAC)
        error("module '%s' has no 'mac' submodule", myStation);

    const char *tmp = myMAC->par("address");
    if (autoSetMACaddress) {
        EVT << "Original MAC address " << tmp;

        uint64 intAddr = 0xAABBCCDDE000L + (subNetIndex << 8) + numHost;
        MACAddress destMACAddress(intAddr);
        EV<< "Changing into new generated MAC address is " << destMACAddress << endl;

        //macModule->address.setAddress(destMACAddress.str());
        myMAC->par("address").setStringValue(destMACAddress.str().c_str());

        EtherMACBase *macModule = check_and_cast<EtherMACBase *>(myMAC);

        macModule->initializeMACAddress();
    }
    else destMACAddress.setAddress(myMAC->par("address"));

    return destMACAddress;
}

