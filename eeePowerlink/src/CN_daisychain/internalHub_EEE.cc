#include "internalHub_EEE.h"
#include "EPLframe_m.h"
#include <cassert>
#include "powerListener.h"

Define_Module(internalHub_EEE);

void internalHub_EEE::initialize(int stage) {

    if (stage==0){
        debug = true;
        inputGateBaseId = gateBaseId("ethg$i");
        outputGateBaseId = gateBaseId("ethg$o");

        powerSignal = registerSignal("powerSignal");
        pkSignal = registerSignal("pk");
        rcvdPwNoEEESignal = registerSignal("rcvdPwNoEEE");

        powerListener *listener = new powerListener(true);
        subscribe(powerSignal, listener);
        powerListener *listener2 = new powerListener(false);
        noLpi_powerSignal = registerSignal("noLpi_powerSignal");
        subscribe(noLpi_powerSignal, listener2);

        powerConsumptionEEE = par("powerConsumptionEEE"); // Power consumption, in LPI
        powerConsumption = par("powerConsumption"); // Power consumption, without EEE

        noEeePowerIncrement = par("noEeePowerIncrement"); // Power consumption increment when active (no EEE)
        eeePowerIncrement = par("eeePowerIncrement"); // Power consumption increment when going active
        pEeePowerIncrement = par("pEeePowerIncrement");
        isStopLPI = 123; // 123 il wakeUpTimer è uno stoplpi

        numHUB = par("numHUB"); // Number of the current HUB
        partialLPI = par("partialLPI").boolValue();

        sleepTime = par("sleepTime"); // Time to go in LPI mode
        quietTime = par("quietTime");
        wakeTime = par("wakeTime");
        refreshTime = par("refreshTime");


        MNaddress = par("MNaddress").stringValue();
        //const char *addressTableFile = par("addressTableFile");
        //if ( addressTableFile && *addressTableFile )

        eplCycleTime = par("eplCycleTime"); // EPL cycle time

        EVT<< "EPL cycle" << eplCycleTime << "'\n"
              "I am the HUB number " << numHUB << "'\n"
              "power Consumption: " << powerConsumptionEEE <<
              "mW. Increment " << eeePowerIncrement << "mW."<< endl;

        numMessages = 0;
        countGates = 0;
        WATCH(numMessages);

        if (gateSize("ethg$i") != gateSize("ethg$o"))
            opp_error("Check connections. Input and output gates number differ!");

        numPorts = gateSize("ethg$i");
        if (numPorts != par("numOutputPorts").longValue() || numPorts > 2 )
            throw cRuntimeError("Something wrong with ports number! Check your configuration");
        if (numPorts < 2){
            EVT << "This is a terminator CN of a daisy chain."<<endl;
            terminator = true;
        }
        hasSubnetworks = par("hasSubnetworks");

        IStartTime = new simtime_t[numPorts];
        IEndTime = new simtime_t[numPorts];

        gateLPI = new short[numPorts];

        for (int i = 0; i < numPorts; i++) {
            gateLPI[i] = 1; // gate is in LPI
            countGates++;
            IStartTime[i] = 0;
            IEndTime[i] = 0;
        }
        EVT<< "Number of ports " << numPorts << " of which in LPI "<<
            countGates << endl;

        countGatesinLPI = countGates;
        checkConnections(true);
        WATCH_MAP(addresstable);

        wakeUpTimer = new cMessage("wakeUp", WAKE_UP);
        startLpiTimer = new cMessage("startLPI", START_LPI);
        timerMsg = new cMessage("startPow", START_POW);

        scheduleAt(simTime(), timerMsg);
    } else if(stage==1){
        myMAC = getMyInterface();
        EVT << "My network address is " << myMAC->getMACAddress() << endl;

        initAddressTable(myMAC->getMACAddress());
        printAddressTable();
    }
}

void internalHub_EEE::handleSelfMsg(cMessage *msg) {
    switch (msg->getKind()) {
        case END_IFG:
            tmp = (EtherFrame*) msg->getContextPointer();
            msg->setContextPointer(NULL);
            delete msg;
            handleAndDispatchFrame(tmp, tmp->getArrivalGate() );
            break;
        case START_POW:
            for (int i = 0; i < numPorts; i++) {
                power = powerConsumptionEEE;
                emit(powerSignal, power);
                power = powerConsumption-noEeePowerIncrement;
                emit(noLpi_powerSignal, power);
            }
            break;
        case START_LPI:
            EV<< "STARTED LPI-";
            EV<< "Starting Low Power Idle on port: ";
            bool isLastPort;
            isLastPort = false;
            for(int i = 0; i < numPorts; i++)
                if(gateLPI[i]==-1 && IStartTime[i]==simTime() ){
                    if(i == numPorts-1)
                        isLastPort = true;
                    gateLPI[i]=1;
                    countGatesinLPI++;
                    countGates++;
                    EV << i << " Gates in LPI  are " << countGates << endl;

                    if(partialLPI)
                        power = -pEeePowerIncrement;
                    else
                        power = -eeePowerIncrement;
                    emit(powerSignal, power);
                    power = -noEeePowerIncrement;
                    emit(noLpi_powerSignal, power);
                }
//            if(isLastPort){// && countGates == numPorts -1){
//                if(partialLPI)
//                    power = -pEeePowerIncrement;
//                else
//                    power = -eeePowerIncrement;
//                emit(powerSignal, power);
//
//                power = -noEeePowerIncrement;
//                emit(rcvdPwNoEEESignal, power);
//            }
            delete msg;
            break;
        case WAKE_UP:
            EV << "Number of LPI gates is " << countGates << endl;
            EV << "STOP LPI-";
            EV<< "Stopping Low Power Idle on port: ";
            int tmpVal;
            tmpVal = *(int*)msg->getContextPointer();

            EV << "Get context pointer " << tmpVal << endl;
            if(partialLPI){
                power = countGatesinLPI * pEeePowerIncrement;
            }
            else
                power = countGatesinLPI * eeePowerIncrement;

            emit(powerSignal, power);
            power = countGatesinLPI * noEeePowerIncrement;
            emit(noLpi_powerSignal, power);

            // we have only two ports!
            // check if the second one is active or not, wake up it
            // and pass up the message!
            for(int i = 0; i < numPorts; i++)
                if( (gateLPI[i]== -2 || gateLPI[i]== -20) && IEndTime[i]==simTime() ){
                    EV << i << "; ";
                    if(countGates>0)
                        countGates--;
                    if(stopIdleMessagePrototype && (gateLPI[i] != -20) )
                        send(stopIdleMessagePrototype->dup(), outputGateBaseId+i);
                    gateLPI[i]=0;
                }
            countGatesinLPI = 0;
            //pass up the message!
            send(stopIdleMessagePrototype->dup(), "tomac$o");

            EV << " Gates in LPI  are " << countGates << endl;

            delete stopIdleMessagePrototype;
            break;
        default:
            EV << "Unknown self-message. Delete, kind: " << msg->getKind() << endl;
            delete msg;
            break;
        }
    }

void internalHub_EEE::handleLowerMsg( cMessage *msg ) {
    if ( dataratesDiffer ) checkConnections(true);

    int arrivalPort = msg->getArrivalGate()->getIndex();
    simtime_t ttx = simTime() - msg->getSendingTime();
    if(gateLPI[arrivalPort] == 1 && !msg->isName("StopIDLE"))
        throw cRuntimeError("Arrivato un frame su una porta in LPI");

    EV<< "Frame " << msg << " with name " << msg->getName() << " arrived on port " << arrivalPort<<
            " port name " << msg->getArrivalGate()->getName() << " tx time : " << ttx << endl;

    if ( msg->isName("LPISignal") || msg->isName("StopIDLE") )
        manageLpiSignals(msg);
    else
        processFrame(msg);
}

void internalHub_EEE::processFrame(cMessage * msg){
    numMessages++;
    emit(pkSignal, msg);

    /*if ( numPorts <= 1 ) {
        delete msg;
        throw cRuntimeError("Why only %d port?", numPorts);
        return;
    }*/

    EtherFrame *frame = (EtherFrame *) msg;
    ASSERT(frame);
    cMessage *tmp = new cMessage("frame", END_IFG);
    tmp->setContextPointer(frame);
    scheduleAt(simTime() + 960e-9, tmp);
    //handleAndDispatchFrame(frame, arrivalPort);
}

void internalHub_EEE::manageLpiSignals(cMessage *msg){
    int arrivalPort = msg->getArrivalGate()->getIndex();

    // from physical we send EtherFrameWithLLC, so we must
    // cast to and then get the encapsulated EPL frame! ;)

    stopIdleMessagePrototype = msg->dup();

    //EtherFrameWithLLC *f= check_and_cast<EtherFrameWithLLC *>(msg);
    EthernetIIFrame *f= check_and_cast<EthernetIIFrame *>(msg);
    EPLframe *req = check_and_cast<EPLframe *>(f->decapsulate()); // EPL frame received

    switch (req->getType()) {
        case LPI_SIGNAL: // LPI received
            numMessages++;
            emit(pkSignal, msg);
            if (gateLPI[arrivalPort] == 1)
                throw cRuntimeError("Gate %d already in LPI.", arrivalPort);

            EV<< "LPI signal on port " << arrivalPort << " trigger LPI mode\n";

            goLPI(arrivalPort);
            delete msg;
            break;
        case STOP_IDLE:
            EV << "Stopping low power IDLE on switch\n";

//            if (countGates == numPorts) {
//                for (int i = 0; i < numPorts; i++) {
//                    if (gateLPI[i]!=1)
//                        throw cRuntimeError("Gate in LPI?? Please check.");
//                    //IEndTime[i] = simTime() + wakeTime;
//                    LPITime = IEndTime[i] - IStartTime[i];
//                    //percH = LPITime/(nodesCycleHUB[numHUB-1]*TPowCycle);
//                    percH = LPITime.dbl()/eplCycleTime.dbl();
//                }
//            }
//            else
//                throw cRuntimeError("difference in number of ports!!!");


            int countNoLpiGates, i;
            countNoLpiGates= 0;
            for (i = 0; i < numPorts; i++)
                if (gateLPI[i]!=1)
                    countNoLpiGates++;

            EV<< "Stop LPI signal on port " << arrivalPort << " exiting LPI mode" <<
            " incremento potenza (calcolato) di " <<
            countGates * (partialLPI?pEeePowerIncrement:eeePowerIncrement) << " mW "<<
            " fissato invece a " << numPorts * (partialLPI?pEeePowerIncrement:eeePowerIncrement) << " mW" << endl;

//            if(!ev.isDisabled() && countNoLpiGates>0){
//                opp_warning("number of port not in LPI are %d", countNoLpiGates);
//            }

            for (int i = 0; i < numPorts; i++) {
                    LPITime = IEndTime[i] - IStartTime[i];
                    percH = LPITime.dbl()/eplCycleTime.dbl();
                }

            wakeUp(-arrivalPort-1); // segno così la porta da cui è arrivata la rich broadcast di stop lpi

            numMessages++;
            emit(pkSignal, msg);

            if (numPorts <= 1) {
                delete msg;
                return;
            }
            delete f;
            delete req;
            break;
        default: // all other message are broadcasted to other nodes!
            throw cRuntimeError("Errore conversione messaggio;");
            break;
        }
}

void internalHub_EEE::goLPI(int portNumber) { // Switch to LPI mode
    gateLPI[portNumber] = -1; // flag x disattivazione porta arrivalPort
    char *new_name;
    asprintf(&new_name, "startLPI_Port%d", portNumber);
    cMessage *tmp = startLpiTimer->dup();
    tmp->setName(new_name);
    scheduleAt(simTime() + sleepTime, tmp);
//    int count = 0;
//    for(int i=0;i<numPorts; i++)
//        if(gateLPI[i] ==-1)
//            count++;
    IStartTime[portNumber] = simTime() + sleepTime;
    EV<< "Starting Low Power Idle on port " << portNumber <<
            " Gates in LPI  are " << countGates <<endl;//<< " requested lpi for " << count << " ports"<< endl;
//    if(count == numPorts - 1 ){
//        EV << "Only one port remained active, I can switch to LPI" << endl;
//        int i=0;
//        for(;i<numPorts; i++)
//            if(gateLPI[i] !=-1){
//                gateLPI[i]=-1;
//                break;
//            }
//        EPLframe *datapacket = new EPLframe("LPISignal", IEEE802CTRL_DATA);
//        datapacket->setType(LPI_SIGNAL);
//        datapacket->setByteLength(0);
//        EtherFrameWithLLC *f= new EtherFrameWithLLC("Lpi Signal for last port");
//        f->encapsulate(datapacket);
//        send(f, "ethg$o", i);//manda LPI request tramite porta di controllo
//        cMessage *tmp = startLpiTimer->dup();
//        tmp->setName("startLPI_lastPort");
//        scheduleAt(simTime() + sleepTime, tmp);
//    }
}

void internalHub_EEE::wakeUp(int portNumber) {
    if(portNumber<0){ // Switch to ACTIVE mode, porte negative sono broadcast!
        EV << "waking up all ports but the number " << -portNumber-1 << endl;
        for(int i = 0; i<numPorts; i++){
            gateLPI[i] = -2; // flag x attivazione lpi su porta
            if(i == (-portNumber-1))
                gateLPI[i] = -20; // flag x attivazione lpi su porta di arrivo
            IEndTime[i] = simTime() + wakeTime;
            EV<< "Exiting Low Power Idle (" << gateLPI[i] << ") on port " << i << endl;
        }
        isStopLPI = 123;
        wakeUpTimer->setContextPointer(&isStopLPI);
        scheduleAt(simTime() + wakeTime, wakeUpTimer);
    }
    else{
        gateLPI[portNumber] = -2; // flag x attivazione porta arrivalPort
        scheduleAt(simTime() + wakeTime, wakeUpTimer);
        IEndTime[portNumber] = simTime() + wakeTime;
        EV<< "Exiting Low Power Idle on port " << portNumber << endl;
    }

}

void internalHub_EEE::finish() {
    simtime_t t = simTime();
    recordScalar("simulated time", t);

    if (t > 0)
        recordScalar("messages/sec", numMessages / t);

    cancelAndDelete(timerMsg);
    cancelAndDelete(wakeUpTimer);
    cancelAndDelete(startLpiTimer);
}

void internalHub_EEE::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage())
        handleSelfMsg(msg);
    else {
        if (msg->arrivedOn("ethg$i") || msg->arrivedOn("tomac$i")) {
            EV<< "Handle lower message." << endl;
            handleLowerMsg(msg);
        }
        else if (msg->getArrivalGateId() == -1)
            throw cRuntimeError("No self message and no gateID?? Check configuration.");
        else
            throw cRuntimeError("Unknown gateID?? Check configuration or override handleMessage().");
    }
}

void internalHub_EEE::checkConnections(bool errorWhenAsymmetric) {
    int numActivePorts = 0;
    double datarate = 0.0;
    dataratesDiffer = false;

    for (int i = 0; i < numPorts; i++) {
        cGate *igate = gate(inputGateBaseId + i);
        cGate *ogate = gate(outputGateBaseId + i);
        if (!igate->isConnected() && !ogate->isConnected())
            continue;
        if (!igate->isConnected() || !ogate->isConnected()) {
            // half connected gate
            if (errorWhenAsymmetric)
                throw cRuntimeError(
                        "The input or output gate not connected at port %i", i);
            dataratesDiffer = true;
            EV<< "The input or output gate not connected at port " << i << ".\n";
            continue;
        }

        numActivePorts++;
        double drate =
                igate->getIncomingTransmissionChannel()->getNominalDatarate();

        if (numActivePorts == 1)
            datarate = drate;
        else if (datarate != drate) {
            if (errorWhenAsymmetric)
                throw cRuntimeError(
                        "The input datarate at port %i differs from datarates of previous ports",
                        i);
            dataratesDiffer = true;
            EV<< "The input datarate at port " << i << " differs from datarates of previous ports.\n";
        }

        cChannel *outTrChannel = ogate->getTransmissionChannel();
        drate = outTrChannel->getNominalDatarate();

        if (datarate != drate) {
            if (errorWhenAsymmetric)
                throw cRuntimeError(
                        "The output datarate at port %i differs from datarates of previous ports",
                        i);
            dataratesDiffer = true;
            EV<< "The output datarate at port " << i << " differs from datarates of previous ports.\n";
        }

        if (!outTrChannel->isSubscribed(POST_MODEL_CHANGE, this))
            outTrChannel->subscribe(POST_MODEL_CHANGE, this);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * METHODS FOR MANAGEMENT OF ADDRESSES
 * TAKEN FROM INET FRAMEWORK
 * o--._._.--> TRAMA
 */

void internalHub_EEE::handleAndDispatchFrame( EtherFrame *frame, cGate *ig ) {

    // handle broadcast frames first
    if ( frame->getDest().isBroadcast() ) {
        EV<< "Broadcasting broadcast frame " << frame << endl;
        broadcastFrame(frame, ig);
        return;
    }

    if (frame->getDest().equals(myMAC->getMACAddress())){
        EVT << "This fame is for me. Passing up" << endl;
        send(frame, "tomac$o");
        return;
    }
    // Finds output port of destination address and sends to output port
    // if not found then broadcasts to all other ports instead
    int outputport = getPortForAddress(frame->getDest());

    /*if(outputport == -2){
        EVT << "MAC address of the receiver " << frame->getDest();
        int result = ( 0x0000000000FFL & frame->getDest().getInt() );
        EV << " Last two digits are " << result;

        cModule *main = simulation.getSystemModule(); // getModuleByPath(destAddress);
        for (cModule::SubmoduleIterator i(main); !i.end(); i++) {
            cModule *submodp = i();
            if( submodp->findSubmodule("CN") != -1){
                if(submodp->getSubmodule("CN")->hasPar("numHost"))
                    if(submodp->getSubmodule("CN")->par("numHost").longValue() == result){
                        EV <<  "Found with par numHost " << submodp->getSubmodule("CN")->par("numHost").longValue() <<
                                " got its Vector addr : " << submodp->getIndex() << endl;
                        result = submodp->getIndex();
                        break;
                    }
            }
            else continue;
        }

        EV << " So the output port number is " << result +1 << endl;
        outputport = result +1;
    }
    // should not send out the same frame on the same ethernet port
    // (although wireless ports are ok to receive the same message)
    if (inputport == outputport) {
        EV << "Output port is same as input port, " << frame->getFullName() <<
        " dest " << frame->getDest() << ", discarding frame\n";
        delete frame;
        return;
    }*/

    if (outputport <= 1 ) { // if this frame is for may daisy chain then the port could only be 1
        EV << "Sending frame " << frame << " with dest address " << frame->getDest() << " to port " << outputport << endl;
        send(frame, "ethg$o", outputport);
    } else {
        EV << "Dest address " << frame->getDest() << " unknown, broadcasting frame " << frame << endl;
        broadcastFrame(frame, ig);
    }

}

void internalHub_EEE::broadcastFrame( EtherFrame *frame, cGate *ig ) {
    EVT << "The input frame arrives from port " << ig->getName() << endl;
    //for ( int i = 0 ; i < numPorts ; ++i )
    if (!terminator){
        if (ig == gate("tomac$i") ){
            send((EtherFrame*) frame->dup(), "ethg$o", 0);
            send((EtherFrame*) frame->dup(), "ethg$o", 1);
        }
        else{
            send((EtherFrame*) frame->dup(), "tomac$o");
            if (ig == gate("ethg$i", 0))
                send((EtherFrame*) frame->dup(), "ethg$o", 1);
            else
                send((EtherFrame*) frame->dup(), "ethg$o", 0);
        }
    }
    else{
        if (ig == gate("tomac$i") )
            send((EtherFrame*) frame->dup(), "ethg$o", 0);
        else
            send((EtherFrame*) frame->dup(), "tomac$o");
    }
    delete frame;
}

void internalHub_EEE::printAddressTable() {
    AddressTable::iterator iter;
    EV<< "Address Table (" << addresstable.size() << " entries):\n";
    for ( iter = addresstable.begin(); iter != addresstable.end() ; ++iter ) {
        EV<< "  " << iter->first << " --> port" << iter->second << endl;
    }
}

int internalHub_EEE::getPortForAddress( MACAddress& address ) {
    MACAddress tmp = address;
    int subnetindex = isMulticast(tmp);
    EV << "This switch " << (hasSubnetworks?"has":"has not") << " subnets, with index "<<subnetindex<<endl;
    if( hasSubnetworks && (subnetindex >= 0) ){
        uint64 result = ( 0xFFFFFFFFFF00L & tmp.getInt() ) + 0xFF;
        tmp = MACAddress(result);
        EV << "Searching for this MAC addr " << tmp.str() << endl;
    }

    printAddressTable();
    AddressTable::iterator iter = addresstable.find(tmp);
    if ( iter == addresstable.end() ) {
        // not found
        EV << "Not found in address table" << endl;
        return -1;
    }
    return iter->second;

}

int internalHub_EEE::isMulticast(MACAddress& addr){
    EV << "Check if this " << addr.str() << " is multicast " << endl;

    int result = ( ( 0x000000000F00L & addr.getInt() ) >> 8 );
    EV << "Subnet index is " << result << "Check " << (result&0xE) << endl;
    if(result == 0xE)
        return -1;
    return result;
}

EtherMACFullDuplex* internalHub_EEE::getMyInterface(){
    EVT << "Get a pointer to my internal Ethernet MAC" << endl;
    cModule *tmp = this->getModuleByPath("^.mac");
    EtherMACFullDuplex *rt = check_and_cast<EtherMACFullDuplex *>(tmp);
    return rt;
}

void internalHub_EEE::initAddressTable(MACAddress addr){

    int result = ( ( 0x000000000F00L & addr.getInt() ) >> 8 );
    EV << "Subnet index is " << result << "Check " << (result & 0xE) << endl;

    if(terminator)
        EVT << "This switch " << (hasSubnetworks?"has":"has not") << " subnets, with index "<< result <<endl;
    else
        EVT << "This switch " << (hasSubnetworks?"has":"has not") << " subnets"<< endl;

    if(hasSubnetworks){
        // forging the multicast address for the rest of the daisy chain
        uint64_t ba = ( ( 0xFFFFFFFFFF00L & addr.getInt() ) | 0xFFL );
        EVT << "the multicast addr of the subnet is " << MACAddress(ba) << endl;
        // redirecting on port 1
        addresstable[MACAddress(ba)] = 1;
    }

    addresstable[MACAddress(MNaddress)] = 0;
}
