
#include "powerListener.h"
#include "Ieee802Ctrl_m.h"
#include "EPL_ManagingNode.h"
#include "EtherApp_m.h"
#include "EPLframe_m.h"
#include <cassert>

Define_Module(EPL_ManagingNode);

void EPL_ManagingNode::initialize(int stage){
    if (stage == 1) {
        reqLength = par("reqLength");
        cycleCount = 0;
        currentHost = 0;
        lpiOnSoA = par("lpiOnSoA").boolValue();
        partialLPI = par("partialLPI").boolValue();
        if(partialLPI && lpiOnSoA)
            throw cRuntimeError("Cannot have partial LPI in this case!");

        powerSignal = registerSignal("powerSignal");
        powerListener *listener = new powerListener(true);
        simulation.getSystemModule()->subscribe(powerSignal, listener);
        noLpi_powerSignal = registerSignal("noLpi_powerSignal");
        powerListener *listener2 = new powerListener(false);
        simulation.getSystemModule()->subscribe(noLpi_powerSignal, listener2);

        hostAwake = false;
        persoPres = false; // Loss of PRes

        // Powerlink cycle parameters
        eplCycleTime = par("eplCycleTime"); // EPL cycle time
        eplPreqPresTime = par("eplPreqPresTime"); // time between Preq and relative Pres
        propagationTime = par("propagationTime"); // propagation time
        wakeTime = par("wakeTime"); // waking time
        //partialWakeTime = par("partialWakeTime");
        TPreq_Pres_MN = par("TPreq_Pres_MN");
        switchDelay = par("switchDelay");

        numberOfSubnetworks = par("numberOfSubnetworks");
        currentSubnet = 1;
        /*if(numberOfSubnetworks>0){
            nodePerSubnet = par("nodePerSubnet");
            currentSubnet = 1;
            midLevelNode = par("midLevelNode");
        }*/

        numberOfSwitch =  simulation.getSystemModule()->par("numberOfSwitch"); //numero di Switch
        numberOfCN = simulation.getSystemModule()->par("numberOfCN"); //numero di slaves

        multiplexed = par("multiplexed");
        if(multiplexed){
            muxEPLtime = par("muxEPLtime");
            const char *str2 = par("muxCN").stdstringValue().c_str();

            // leggere nodi multiplexati
            char str[10];
            strcpy(str, str2);
            char *pch;
            EV << "Splitting string " << str << " into tokens" << endl;
            pch = strtok (str,",");
            while (pch != NULL){
              EV << pch << endl;
              muxCN.push_back(atoi(pch));
              pch = strtok (NULL, " ,.-");
            }

            int rows = muxEPLtime / eplCycleTime;
            int adj_cols = ceil( (double) muxCN.size() / rows );
            numPollingPerCycle = adj_cols + numberOfCN - muxCN.size();
            EV << "Building table - rows "<< rows << " cols " << numPollingPerCycle << endl ;
            //cycle_table[0].resize( numberOfCN - muxCN.size() + cols );
            cycle_table.resize(rows, std::vector<int>(numPollingPerCycle, 0) );
            for(int i = 0; i < rows ; i++)
                for(int j = 0; j < (numberOfCN - muxCN.size()); j++)
                    cycle_table[i][j] = j;

            //muxCN.resize(rows,0);
            for(int i = 0; i < rows ; i++)
                for(int j = 0; j < adj_cols; j++)
                    cycle_table[i][numberOfCN - muxCN.size() + j] =
                            ((i*adj_cols + j)<muxCN.size()) ? muxCN[i*adj_cols + j] : -10;

            read_cycle_table();
        }

        cycle_count = 0;
        numPolledSlave = 0;

        double datarate = simulation.getSystemModule()->par("dr");
        Ttx = ((reqLength<46 ? 46 : reqLength) + 30) * 8 / datarate; // payload + 6+6+2+4+4+8

        // T for SoC propagation
        Tsoc = (numberOfSwitch + 1) * ( 0.96e-6 + Ttx + switchDelay+ propagationTime); // tempo necessario perché uno Soc raggiunga l'ultimo nodo
        Tsi = Tsoc + (numberOfSwitch + 2) * wakeTime;

        // timeout for Pres
        if(partialLPI)
            preqTimeOutTime = 2 * Tsoc + eplPreqPresTime + wakeTime + 1e-6; //piccolo tempo di guardia
        else
            preqTimeOutTime = 2 * Tsoc + eplPreqPresTime;

        EV << "Datarate: " << datarate <<
            " Tempo tx: " << Ttx <<
            " Tsoc: " << Tsoc <<
            " T stop idle: " << Tsi <<
            " timeout pres: " << preqTimeOutTime << endl;

        localSAP = ETHERAPP_CLI_SAP;
        remoteSAP = ETHERAPP_SRV_SAP;

        seqNum = 0;

        // statistics
        packetsSent = packetsReceived = 0;
        sentPkSignal = registerSignal("sentPk");
        rcvdPkSignal = registerSignal("rcvdPk");

        EVT << "Submodule " << this->getParentModule()->getSubmodule("lpi_client")->getFullPath() << endl;
        this->getParentModule()->getSubmodule("lpi_client")->subscribe("LPI_indication", this);

        bool registerSAP = par("registerSAP");
        if (registerSAP)
            registerDSAP(localSAP);

        simtime_t startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime != 0 && stopTime <= startTime)
            error("Invalid startTime/stopTime parameters");

        socTimer = new cMessage("startPowerLinkCycle", SEND_SOC);
        wakeUpTimer = new cMessage("StopIDLE", WAKE_UP);
        preqTimer = new cMessage("Preq Send", POLLING);
        preqTimeoutTimer = new cMessage("Preq timeout", PREQ_TIMEOUT);
        soaTimer = new cMessage("SoA", SEND_SOA);

        scheduleAt(startTime, wakeUpTimer);

        WATCH(packetsSent);
		WATCH(packetsReceived);
		WATCH(seqNum);
    }
}

void EPL_ManagingNode::handleSelfMsg(cMessage *msg) {
	switch (msg->getKind()) {
		case WAKE_UP: // Wake Up the CNs
			assert(!hostAwake);
			TimeEPL = simTime();
			// Stop the IDLE fase
			sendPacket(STOP_IDLE, -1);
			EV << "Fine fase di IDLE\n";
			scheduleAt(simTime() + eplCycleTime, wakeUpTimer);
			// Start of EPL cycle
			scheduleAt(simTime() + Tsi, socTimer);
			break;

		case SEND_SOC: // SoC transmission
	        if(!hostAwake)
	            throw cRuntimeError("Host is not awake!");

		    cycleCount++; // start new cycle
			EV << " Sending SoC broadcast! " << cycleCount << endl;
			sendPacket(SOC, -1); // Inizio del Ciclo Powerlink
			scheduleAt(simTime() + Tsoc + 1e-6, preqTimer); // Wait SoC propagation time before starting CNs polling
			TimeEPL = simTime();
			break;

		case POLLING: // Preq polling
			if (persoPres) { // Loss of PRes
				EV << "Pres got lost! " << currentHost << endl;
			}
			EV << "Sending Preq. Dest: Host" << currentHost << endl;
			sendPacket(PREQ, currentHost);
			EV << "Preq trasmesso, avvio il timeout \n";

			scheduleAt(simTime() + preqTimeOutTime, preqTimeoutTimer);
			break;

		case PREQ_TIMEOUT:
			if (!multiplexed){
                if(currentHost == numberOfCN - 1) // Wait the timeout and send the SoA if polling is completed
                    scheduleAt(simTime(), soaTimer);
                else { // Wait the timeout and send the next PReq
                    persoPres = true;
                    currentHost++;
                    EV << "Timeout fired. Next Host " << currentHost << "\n";
                    scheduleAt(simTime(), preqTimer);
                }
			}
			else{
                if(numPolledSlave == numPollingPerCycle-1) // Wait the timeout and send the SoA if polling is completed
                    scheduleAt(simTime(), soaTimer);
                else { // Wait the timeout and send the next PReq
                    persoPres = true;
                    currentHost = cycle_table[cycle_count][++numPolledSlave];
                    EV << "Timeout fired. Next Host " << currentHost << "\n";
                    scheduleAt(simTime(), preqTimer);
                }
			}
			break;

		case SEND_SOA: // SoA trasmission
		    TimeEPL = simTime() - TimeEPL;
			EV << "Durata fase isocrona: " << TimeEPL.dbl() * 1000000 << "us\n";
            if(multiplexed) {
                cycle_count = (cycle_table.size()-cycle_count>1) ? cycle_count+1 : 0;
                EV << "Updating cycle count " << cycle_count<<endl;
                numPolledSlave=0;
            }

			if(lpiOnSoA){
                SoAHost = (rand() % numberOfCN); // Random Host between 0 and numberOfCn
                TimeAsync = simTime();
                if(!hostAwake)
                    throw cRuntimeError("Host is not awake!");

                sendPacket(SOA, SoAHost);
			}
			else{
			    EV << "No Async request in partial LPI, starting with idle."<< endl;
			    sendPacket(SOA, -1);
			    startIdle();
			}
			currentHost = 0; // First Host for the next EPL cycle
			break;

		default:
			opp_error("Non so");
			break;
	}
}

void EPL_ManagingNode::handleMessage(cMessage *msg){
    if (msg->isSelfMessage()) {
        handleSelfMsg(msg);
    }
    else {
        if(!hostAwake)
            throw cRuntimeError("Host is not awake!");

        EV << "Received packet `" << msg->getName() << "'\n";

        packetsReceived++;
        emit(rcvdPkSignal, msg);

        EPLframe *req = check_and_cast<EPLframe *>(msg); // EPL frame received
        EV<< "tipo " << req->getType() << " host " << req->getHost()<<
                " current host is " << currentHost << "\n";
        if( req->getType() == PRES &&  req->getHost() == currentHost) // Pres received
        {
            persoPres = false;

            EV<< "PRes_Host received from " << currentHost << " deleting timer \n";
            cancelEvent(preqTimeoutTimer); // reset timeout

            if (!multiplexed) {
                if (currentHost == numberOfCN - 1) { // Send the SoA if polling is completed
                //cancelEvent(preqTimeoutTimer); // reset timeout
                    EV<< "Il polling è terminato, procedo con la fase Asincrona \n";
                    scheduleAt(simTime(), soaTimer);
                }
                else {                // Send the next PReq
                    currentHost++;
                    EV<< "Next Host "<< currentHost << "\n";
                    scheduleAt(simTime() + TPreq_Pres_MN, preqTimer);
                }
            }
            else{
                numPolledSlave++;
                if(numPolledSlave == numPollingPerCycle || cycle_table[cycle_count][numPolledSlave]==-10) { // Send the SoA if polling is completed
                    //cancelEvent(preqTimeoutTimer); // reset timeout
                    EV<< "Il polling è terminato, procedo con la fase Asincrona \n";
                    scheduleAt(simTime(), soaTimer);
                }
                else {                // Send the next PReq
                    currentHost = cycle_table[cycle_count][numPolledSlave];
                    EV<< "Next Host "<< currentHost << "\n";
                    scheduleAt(simTime() + TPreq_Pres_MN, preqTimer);
                }
            }
        }
        else if(req->getType() == ASYNC)        // SoA polling
        {
            TimeAsync = simTime() - TimeAsync;
            EV << "Durata fase asincrona: " << TimeAsync.dbl()*1000000 << "us\n";
            startIdle();
        }
        delete msg;
    }
}

void EPL_ManagingNode::startIdle() {
    EV<< "Start of IDLE phase. Sending LPI.request()"<<endl;
    EV<< "Generating LPI packet `LPISignal'\n";

    // LPI signal Generation
    EPLframe *datapacket = new EPLframe("LPISignal", IEEE802CTRL_DATA);
    datapacket->setType(LPI_SIGNAL);
    datapacket->setByteLength(0);
    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setSsap(localSAP);
    etherctrl->setDsap(remoteSAP);
    datapacket->setControlInfo(etherctrl);
    emit(sentPkSignal, datapacket);
    send(datapacket, "ctrlOut");

    EV << "Durata fase idle: " << (eplCycleTime - TimeAsync - TimeEPL).dbl()*1000000 << "us\n";
}

void EPL_ManagingNode::receiveSignal(cComponent* source, simsignal_t signalID,
        const char* s) {

    EV << "Received signal from LPI client " << source->getFullPath() <<
            ", signal --> " << s << endl;

    if( ! this->getParentModule()->getSubmodule( source->getName() ) ) // controllo che il segnale arrivi dal "mio" lpi_client
        throw cRuntimeError("Not a submodule of this compound module (%s)!",this->getFullName());

    if(strcmp(s,"AWAKE") == 0){
        EV << "Received awake signal. Node is now awake\n";
        hostAwake = true;
    }
    else if(strcmp(s,"partialAWAKE") == 0){
            EV << "Now in partial LPI\n";
            hostAwake = true;
            if(!partialLPI)
                throw cRuntimeError("Module params not consistent!");
    }
    else if(strcmp(s,"LPI") == 0){
        EV << "Now in LPI\n";
        hostAwake = false;
    }
}

void EPL_ManagingNode::registerDSAP(int dsap){
    EV << getFullPath() << " registering DSAP " << dsap << "\n";

    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setDsap(dsap);
    cMessage *msg = new cMessage("register_DSAP", IEEE802CTRL_REGISTER_DSAP);
    msg->setControlInfo(etherctrl);
    send(msg, "out");
}

void EPL_ManagingNode::sendPacket(int type, int dest){
    seqNum++;
    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    std::string msgname;
    switch ( type ) {
    case SOC: // SoC
        msgname = "SoC";
        etherctrl->setDest(MACAddress::BROADCAST_ADDRESS);
        break;
    case PREQ: // Preq
        destMACAddress = resolveDestMACAddress(dest);
        etherctrl->setDest(destMACAddress);
        msgname = nextMsgNameForPreq(dest, destMACAddress);
        break;
    case SOA: // SoA
        char *tmp;
        asprintf(&tmp,"SoA_Host%d", dest);
        msgname = std::string(tmp);
        etherctrl->setDest(MACAddress::BROADCAST_ADDRESS);
        break;
    case STOP_IDLE: // StopIdle
        msgname = "StopIDLE";
        etherctrl->setDest(MACAddress::BROADCAST_ADDRESS);
        break;
    }

    EPLframe *datapacket = new EPLframe(msgname.c_str(), IEEE802CTRL_DATA);
    datapacket->setByteLength(reqLength);
    datapacket->setType(type);
    datapacket->setMp(cycleCount);
    datapacket->setHost(dest);
    etherctrl->setSsap(localSAP);
    etherctrl->setDsap(remoteSAP);
    datapacket->setControlInfo(etherctrl);

    EV << "Generating packet `" << msgname << "' with length " << reqLength<<
            " to destination host " << datapacket->getHost() << "\n";
    if(type == STOP_IDLE){
        if(partialLPI){
            EV << "setting partial LPI flag" << endl;
            datapacket->setPartialLPI(true);
        }
        send(datapacket, "ctrlOut");
    }
    else{
        emit(sentPkSignal, datapacket);
        send(datapacket, "out");
    }
    packetsSent++;
}

inline std::string EPL_ManagingNode::nextMsgNameForPreq(int nextHostNumber, MACAddress macad){
    /*
     * Bisogna fissare delle convenzioni!
     * la rete sarà suddivisa in nodi "sciolti" e alcune sottoreti, che faranno capo ad uno
     * switch ciascuna.
     * numberOfSubnetworks: numero di sottoreti da gestire, se 0 allora ci saranno solo nodi sciolti
     * i nodi singoli si chiameranno genericamente Host0, Host1, ecc...
     * gli altri saranno identificati genericamente come SubNet(i)_Host(k)
     *
     */
    char *tmp;
    if(numberOfSubnetworks == 0){
        if(asprintf(&tmp, "Preq_Host%d", nextHostNumber) < 0)
            return NULL;
    }
    else{
        currentSubnet = ( ( 0x000000000F00L & macad.getInt() ) >> 8 );
        if ( currentSubnet > numberOfSubnetworks){
            if(asprintf(&tmp, "Preq_Host%d", nextHostNumber) < 0)
                return NULL;
        }
        else if(asprintf(&tmp, "Preq_Subnet%d_Host%d", currentSubnet, nextHostNumber) < 0)
                return NULL;
    }
    return tmp;

}

void EPL_ManagingNode::finish(){
    cancelAndDelete(preqTimer);
    cancelAndDelete(socTimer);
    cancelAndDelete(wakeUpTimer);
    cancelAndDelete(soaTimer);
    cancelAndDelete(preqTimeoutTimer);
}


MACAddress EPL_ManagingNode::resolveDestMACAddress(int dest) {
    MACAddress destMACAddress;

        // destAddress represent the value of numHost parameter, searching
    // cerco il modulo il cui numHost è uguale al numero sequenziale del preq!
    if (dest >= 0) {
        cModule *main = simulation.getSystemModule(); // getModuleByPath(destAddress);

        for (cModule::SubmoduleIterator i(main); !i.end(); i++) {
            cModule *submodp = i();
            //EV << submodp->getFullName();
            if( submodp->findSubmodule("CN") != -1){
                if(submodp->getSubmodule("CN")->hasPar("numHost"))
                    if(submodp->getSubmodule("CN")->par("numHost").longValue() == dest){
                        destMACAddress.setAddress(submodp->getSubmodule("mac")->par("address"));
                        EV <<  " with par numHost " << submodp->getSubmodule("CN")->par("numHost").longValue() <<
                                " got its MAC addr : " << destMACAddress.str() << endl;
                        break;
                    }
            }
            else continue;
        }
    }

    return destMACAddress;
}
void EPL_ManagingNode::read_cycle_table(){
    EV << "Cycle table\n________________________\n";
    int rows = cycle_table.size();
    int cols = cycle_table[0].size();
    for(int i = 0; i < rows ; i++){
        EV << i << " | ";
        for(int j = 0; j< cols; j++)
            EV << cycle_table[i][j] << " ";
        EV << "|\n";
    }
}
