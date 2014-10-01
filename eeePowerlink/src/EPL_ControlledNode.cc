
#include "EPL_ControlledNode.h"

#include "EtherApp_m.h"
#include "Ieee802Ctrl_m.h"
#include "EPLframe_m.h"
#include "powerListener.h"
#include <cassert>
#include "EtherMACBase.h"

Define_Module(EPL_ControlledNode);

void EPL_ControlledNode::initialize() {

    debug = true;
    hostAwake = false;
    onlyRx = false; //flag x capire se sono in LPI parziale attivo
    lpiOnSoA = par("lpiOnSoA").boolValue();
    partialLPI = par("partialLPI").boolValue();
    if(partialLPI && lpiOnSoA)
        throw cRuntimeError("Cannot have partial LPI in this case!");

    reqLength = par("reqLength").longValue();
    numHost = par("numHost").longValue();

    // Subscribing to LPI_indication signal to track LPI state change
    EVT << "Submodule " << this->getParentModule()->getSubmodule("lpi_client")->getFullPath() << endl;
    this->getParentModule()->getSubmodule("lpi_client")->subscribe("LPI_indication", this);

    localSAP = ETHERAPP_SRV_SAP;
    remoteSAP = ETHERAPP_CLI_SAP;

    seqNum = 0;
    WATCH(seqNum);

    // statistics
    packetsSent = packetsReceived = 0;
    sentPkSignal = registerSignal("sentPk");
    rcvdPkSignal = registerSignal("rcvdPk");

    TPreq_Pres_CN = par("TPreq_Pres_CN");
    sleepTime = par("sleepTime"); // Time to go in LPI mode
    quietTime = par("quietTime"); // Refresh LPI signal period
    wakeTime = par("wakeTime"); // Wake up time
    //partialWakeTime = par("partialWakeTime"); // Wake up time

    subNetIndex = par("subNetIndex");
    autoSetMACaddress = par("autoSetMACaddress");

    eplCycleTime = par("eplCycleTime"); // EPL cycle time
    propagationTime = par("propagationTime"); // propagation time

    lpiTimer = new cMessage("SoaGoLPI",START_LPI);
    wakeUpTimer = new cMessage("wakeUp", WAKE_UP);
    awakeTimer = new cMessage("wake", WAKED_UP);
    quietTimer = new cMessage("quiet",QUIET);
    presTimer = new cMessage("SendPres", SEND_PRES);

    sprintf(SoAString, "SoA%d", numHost);
    EVT<< "My number is `" << numHost << "'\n";

    WATCH(packetsSent);
    WATCH(packetsReceived);

    bool registerSAP = par("registerSAP");

    if ( registerSAP ) registerDSAP(localSAP);

    if(subNetIndex >= 0)
        setMACAddress(subNetIndex);


}

void EPL_ControlledNode::receiveSignal(cComponent* source, simsignal_t signalID,
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
            EV << "Now in partial LPI\n";
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

void EPL_ControlledNode::handleSelfMsg( cMessage *msg ) {

    switch ( msg->getKind() ) {
        case SEND_PRES: // Send PRes after TPreq_Pres_CN
            if(onlyRx)
                throw cRuntimeError("Cannot send pres, host yet in partial LPI");
            EVT<< "sending PRES\n";
            sendPacket(datapacket, srcAddr);
            if( !lpiOnSoA) { // vado in LPI dopo ilPres
                sendLPISignal(1); // Switch to LPI mode
            }
            break;

        case START_LPI:
            if(!hostAwake)
                throw cRuntimeError("Host is not awake!");
            EV << "Host is awake? " << (hostAwake?"true":"false") << endl;
            //hostAwake = false;
            IStartTime = simTime();
            EVT << "Going in LPI " << IStartTime << "\n";
            scheduleAt(simTime() + quietTime, quietTimer);
            break;
        case QUIET:
            EVT << "Sending periodic LPI Signal\n";
            scheduleAt(simTime() + quietTime, quietTimer);
            EVT << "Quiet\n";
            break;
        case WAKE_UP:
            EVT << "Waking up\n";
            IEndTime = simTime();
            if(quietTimer->isScheduled())
                cancelEvent(quietTimer);

            EVT << "exit LPI " << IEndTime << "\n";

            scheduleAt(simTime() + wakeTime, awakeTimer);
            break;
        case WAKED_UP:
            EVT << "Tempo trascorso in LPI dall'Host " << numHost << "Tidle = "
                                << (IEndTime - IStartTime)<<endl;
            break;
        }
}

void EPL_ControlledNode::handleMessage( cMessage *msg ) {
    if ( msg->isSelfMessage() ) {
        handleSelfMsg(msg);
    }
    else {
        EPLframe *req = check_and_cast<EPLframe *>(msg); // EPL frame received
        Ieee802Ctrl *ctrl;

        switch ( req->getType() ) {
            case SOC:
                EVT << "Arrived SoC" << endl;
                if(quietTimer->isScheduled())
                    cancelEvent(quietTimer);
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
                if(onlyRx){
                    EV << "Before transmitting our response, we must first activating from PLPI"<< endl;
                    sendLPISignal(2);
                    scheduleAt(simTime() + TPreq_Pres_CN + wakeTime + 500e-9, presTimer);
                }
                else
                    scheduleAt(simTime() + TPreq_Pres_CN, presTimer);
                break;

            case SOA: // SoA received
                if(!hostAwake)
                    throw cRuntimeError("Host is not awake!");
                if(!partialLPI){
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
                    sendLPISignal(0); // Switch to LPI mode
                } else{  // SoA not indexed to me, switch to LPI mode, provided I was not already there
                    if(lpiOnSoA)
                        sendLPISignal(0); // Switch to LPI mode
                    delete msg;
                }}
                else {
                    sendLPISignal(0);
                }
                break;

            case STOP_IDLE: // wake up if stopIDLE received and it's our cycle
                scheduleAt(simTime(), wakeUpTimer);
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

void EPL_ControlledNode::sendLPISignal(int flag) {

    if( flag == 0){
        EVT<< "Generating LPI packet `LPISignal'\n";
        // LPI signal Generation
        datapacket = new EPLframe("LPISignal", IEEE802CTRL_DATA);
        datapacket->setType(LPI_SIGNAL);
        datapacket->setByteLength(0);

        Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
        etherctrl->setSsap(localSAP);
        etherctrl->setDsap(remoteSAP);
        datapacket->setControlInfo(etherctrl);

        emit(sentPkSignal, datapacket);
        send(datapacket, "ctrlOut");//manda LPI request tramite porta di controllo
        scheduleAt(simTime(), lpiTimer);
    }
    else if ( flag == 1){
        EV << " partial is " << partialLPI << " and onlyRx " << onlyRx << endl;
        if(!partialLPI || onlyRx)
            throw cRuntimeError("Something strange");
        datapacket = new EPLframe("returnPLPI", IEEE802CTRL_DATA);
        datapacket->setType(RETURN_PLPI);
        datapacket->setByteLength(0);

        Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
        etherctrl->setSsap(localSAP);
        etherctrl->setDsap(remoteSAP);
        datapacket->setControlInfo(etherctrl);

        emit(sentPkSignal, datapacket);
        send(datapacket, "ctrlOut");//manda LPI request tramite porta di controllo
        //scheduleAt(simTime(), lpiTimer);
    }
    else if(flag == 2){
        if(!partialLPI || !onlyRx)
            throw cRuntimeError("Something strange");
        EPLframe *fa = new EPLframe("FullActivate", IEEE802CTRL_DATA);
        fa->setType(EXIT_PLPI);
        fa->setByteLength(0);

        Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
        etherctrl->setSsap(localSAP);
        etherctrl->setDsap(remoteSAP);
        fa->setControlInfo(etherctrl);

        emit(sentPkSignal, fa);
        send(fa, "ctrlOut");//manda LPI request tramite porta di controllo
        //scheduleAt(simTime(), lpiTimer);
    }
}

void EPL_ControlledNode::sendPacket( EPLframe *datapacket, const MACAddress& destAddr ) {
    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setSsap(localSAP);
    etherctrl->setDsap(remoteSAP);
    etherctrl->setDest(MACAddress::BROADCAST_ADDRESS);
    if ( datapacket->getType() == ASYNC )
        etherctrl->setDest(destAddr);
    datapacket->setControlInfo(etherctrl);
    emit(sentPkSignal, datapacket);
    send(datapacket, "out");
    packetsSent++;
}

void EPL_ControlledNode::registerDSAP( int dsap ) {
    EVT<< getFullPath() << " registering DSAP " << dsap << "\n";

    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setDsap(dsap);
    cMessage *msg = new cMessage("register_DSAP", IEEE802CTRL_REGISTER_DSAP);
    msg->setControlInfo(etherctrl);

    send(msg, "out");
}

void EPL_ControlledNode::finish() {
    cancelAndDelete(awakeTimer);
    cancelAndDelete(wakeUpTimer);
    cancelAndDelete(lpiTimer);
    cancelAndDelete(quietTimer);
    cancelAndDelete(presTimer);
}

MACAddress EPL_ControlledNode::setMACAddress(int subNetIndex) {

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

