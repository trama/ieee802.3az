//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "LPIclient_100baseTX.h"
#include "hello_m.h"
#include <cassert>

Define_Module(LPI_client_100baseTX);

void LPI_client_100baseTX::initialize( int stage ) {
    if ( stage == 0 ) {

        debug = par("debug").boolValue();

        // Signals used to track power consumption
        powerSignal = registerSignal("powerSignal");
        noLpi_powerSignal = registerSignal("noLpi_powerSignal");

        // Registering an LPI_indication signal to track LPI state change
        lpiSignal = registerSignal("LPI_indication");
        // Registering a signal to track LPI state change time
        lpi_time = registerSignal("lpi_time");
        partial_lpi_time = registerSignal("partial_lpi_time");

        //thisMN = par("iAmTheMn").boolValue();
        eee_listener = new powerListener(true);
        subscribe(powerSignal, eee_listener);
        std_listener = new powerListener(false);
        subscribe(noLpi_powerSignal, std_listener);

        // FìSignal for communication with the MAC
        macSignals = registerSignal("MAC-LPIClient signals");
        this->getParentModule()->subscribe(macSignals, this);
        partialLPI = par("partialLPI").boolValue();

        lpi2macControl = findGate("lpi2macControl");
        mac2lpiControl = findGate("mac2lpiControl");

        // Some self-messages for LPI management
        startLpiTimer = new cMessage("startLPI", START_LPI);
        quietTimer = new cMessage("refreshCycle", QUIET);
        refreshTimer = new cMessage("refreshCycle", REFRESH);
        wakeUpTimer = new cMessage("wakeUp", WAKE_UP);
        wakedUpTimer = new cMessage("awake", WAKED_UP);
        lpiMessage = new cMessage("LPI", WAKED_UP);

        // EEE Time params
        sleepTime = par("sleepTime"); // Time to go in LPI mode
        quietTime = par("quietTime");
        wakeTime = par("wakeTime");
        partialWakeTime = par("partialWakeTime");
        partialSleepTime = par("partialSleepTime");
        refreshTime = par("refreshTime");

        // EEE power params
        totPower = par("powerConsumption"); // Power consumption when active
        lpiPower = par("powerConsumptionEEE"); // LPI Power consumption with EEE
        pLpiPower = par("powerConsumptionPEEE"); // LPI Power consumption with partial EEE
        eeePowerIncrement = par("eeePowerIncrement");
        noEeePowerIncrement = par("noEeePowerIncrement");
        eeeTransitionPower = par("eeeTransitionPower");
        pEeePowerIncrement = par("pEeePowerIncrement");
        pEeeTransitionPower = par("pEeeTransitionPower");

        queue = new cQueue("Lpi packet queue");
    }
    if (stage == 1){
        //Nodes starts in the active state
        //if(partialLPI)
            emit(powerSignal, totPower); // save default power in LPI
        //else
        //    emit(powerSignal, lpiPower); // save default power in LPI
        emit(noLpi_powerSignal, totPower);

        // I need to discover my neighbour!
        findMyNeighbour(true, NULL);


    }
}

void LPI_client_100baseTX::handleSelfMsg(cMessage *msg) {
	switch (msg->getKind()) {
		case START_LPI:
	        active = false;
			EVT << "START LPI - go LPI " << simTime() << "\n";
			emit(noLpi_powerSignal, -noEeePowerIncrement);
			if(partialLPI)
			    emit(powerSignal, -pEeePowerIncrement + eeeTransitionPower); //se sono un normale nodo calo di 200 e torno a 250
			else
			    emit(powerSignal, -eeePowerIncrement + eeeTransitionPower); // se non sono in LPI parz
			                        // oppure sono il master calo di 450, e torno a 50 mW

			emit(lpiSignal, "LPI");
			IStartTime = simTime();
			PITotTime = PITotTime + simTime() - PIStartTime;
			emit(partial_lpi_time, PITotTime);
			scheduleAt(simTime() + quietTime, quietTimer);
			break;
		case QUIET:
            assert(!active);
            EVT << "Sending periodic LPI Refresh Signal!"<<endl;
            scheduleAt(simTime() + refreshTime, refreshTimer);
            if(partialLPI)
                emit(powerSignal, pEeePowerIncrement - eeeTransitionPower); //se sono un normale nodo calo di 200 e torno a 250
            else
                emit(powerSignal, eeePowerIncrement - eeeTransitionPower); // se non sono in LPI parz
                                    // oppure sono il master calo di 450, e torno a 50 mW
            EVT << "Rescheduling periodic LPI Refresh Signal!"<<endl;
            scheduleAt(simTime() + quietTime, quietTimer);
            EV << "Quiet\n";
            break;
		case REFRESH:
		    if(partialLPI)
                emit(powerSignal, -pEeePowerIncrement + eeeTransitionPower); //se sono un normale nodo calo di 200 e torno a 250
            else
                emit(powerSignal, -eeePowerIncrement + eeeTransitionPower); // se non sono in LPI parz
                                    // oppure sono il master calo di 450, e torno a 50 mW

		    break;
		case WAKE_UP:
		    EV << "Exiting Low Power Idle. Waking up\n";
			IEndTime = simTime();

            if(partialLPI){
                emit(powerSignal, pEeeTransitionPower); // resto a 50 mW
                PIStartTime = simTime();
            }
            else{
                //EV << "I am the MN so I does not use partial LPI" << endl;
                emit(powerSignal, eeeTransitionPower); // resto a 50 mW
            }
            emit(lpi_time, IEndTime - IStartTime);
			scheduleAt(simTime() + wakeTime, wakedUpTimer);
			break;

		case WAKED_UP:
            active = true;
			EV << "STOP LPI -- Tempo trascorso in LPI T_idle = " << (IEndTime - IStartTime) << endl;

			if(partialLPI){
			    emit(lpiSignal, "partialAWAKE");
			    emit(powerSignal, pEeePowerIncrement - pEeeTransitionPower); // vado a 250 mW
			}
			else{
			    emit(lpiSignal, "AWAKE");
			    emit(powerSignal, eeePowerIncrement - eeeTransitionPower); // vado a 500 mW
			}
			emit(noLpi_powerSignal, noEeePowerIncrement);

			lpiMessage->setName("TX attivato");
			lpiMessage->setKind(WAKED_UP);
			send(lpiMessage->dup(), lpi2macControl);
			break;
		case START_LPITX:
		    EV << "returning in partial LPI" << endl;
	        if(partialLPI){
	            emit(lpiSignal, "partialAWAKE");
	            emit(powerSignal, -2*( totPower - pLpiPower)); // ritorno a 250 mW
	            PIStartTime = simTime();
	        }
	        else
	            throw cRuntimeError("Qualcosa non va?!");
	        break;
		case FULL_ACTIVE:
            EV << "Full activation" << endl;
            if(!queue->empty()){
                if(queue->length()>1)
                    throw cRuntimeError("LpiClient Queue length is wrong");
                cMessage *tmp = check_and_cast<cMessage *>( queue->pop() );
                EV << "Full activation of link" << endl;
                delete tmp;
                //sendDown( tmp );
            }
            if(partialLPI){
                emit(lpiSignal, "AWAKE");
                emit(powerSignal, 2*( totPower - pLpiPower) ); // cresco di 500 mW -> 750 mW
                EV << "Incremento la potenza solo all'host del doppio, per simulare l'incremento anche allo switch." << endl;
                // aggiungo al client anche la potenza dell'attivazione dello switch!
                // da correggere!
                PITotTime = simTime() - PIStartTime; // segno quanto tempo sono stato in eee2 finora
            }
            else
                throw cRuntimeError("Errore in lpiclient full activation!");
            break;
	}
}

void LPI_client_100baseTX::handleMac2LpiControl(cMessage *msg){
    EVT << "Received msg " << msg->getClassName() << msg->getFullPath() << endl;

    if (msg->getKind() == START_LPI) { // START ENETRING LPI
        EVT << "Received an LPI.request()" << endl;
        if(partialLPI)
            emit(powerSignal, -pEeeTransitionPower); // resto a 250 mW
        else
            emit(powerSignal, -eeeTransitionPower); // resto a 500 mW

        emit(lpiSignal, "startLPI");
        scheduleAt(simTime() + sleepTime, startLpiTimer);
        delete msg;
        EV << "Triggering Low Power Idle\n";
    }
    /*else if(msg->getKind() == EXIT_PLPI) {
        EVT << "We must totally wake Up from partial LPI." << endl;
        fullActive(req);
        cMessage *tmp =  req->dup(); //duplica il messaggio
        tmp->setControlInfo(msg->removeControlInfo()); //bisogna attaccarci la controlInfo
        queue->insert(tmp);
        EVT << "Queue length " << queue->length() << endl;
    }
    else if(msg->getKind() == RETURN_PLPI) {
        EVT << "Return to partial LPI." << endl;
        EVT << "Received an LPI.request()" << endl;
        EV << "Triggering partial Low Power Idle\n";
        cMessage *tmp = new cMessage("returnLPI", START_LPITX);
        scheduleAt(simTime() + partialSleepTime, tmp);
    }*/
    else
    if(msg->getKind() == WAKE_UP) {
        EVT << "Link must wake Up." << endl;
        wakeUp(msg);
    }
    else if (msg->getKind() == 12346) // msg for registering the neighbor lpi client
        findMyNeighbour(false, msg);

}

void LPI_client_100baseTX::wakeUp(cMessage *frame) {
	if(quietTimer->isScheduled())
		cancelEvent(quietTimer);

	if(startLpiTimer->isScheduled()){
	    EVT << "The link was going to sleep. Interrupting sleep procedure, and now resuming. " << endl;
	    cancelEvent(startLpiTimer);
        lpiMessage->setName("TX attivato");
        lpiMessage->setKind(WAKED_UP);
        sendDelayed(lpiMessage->dup(), wakeTime, lpi2macControl);
	}else
	    scheduleAt(simTime(), wakeUpTimer);
	delete(frame);

}

void LPI_client_100baseTX::fullActive(cMessage *frame) {
    cMessage *timer = new cMessage("FullActivation", FULL_ACTIVE);
    if(timer->getContextPointer() == NULL)
        timer->setContextPointer(frame);
    scheduleAt(simTime() + partialWakeTime, timer);
}

void LPI_client_100baseTX::finish() {
    cancelAndDelete(quietTimer);
    cancelAndDelete(startLpiTimer);
    cancelAndDelete(wakeUpTimer);
    cancelAndDelete(wakedUpTimer);
    cancelAndDelete(lpiMessage);
    cancelAndDelete(refreshTimer);
    if(!queue->empty())
        queue->clear();
    queue->~cQueue();
    eee_listener->finish();
    std_listener->finish();
}

void LPI_client_100baseTX::findMyNeighbour(bool request, cMessage *reply){

    if(request){
        EVT << "Searching for my neighbor LPI CLient " << endl;
        Hello *msg = new Hello("HELLO", 12345);
        send(msg, lpi2macControl);
    }
    else{
        EVT << "Neighbor replied with " << reply->getName() << endl;
        Hello *msg = dynamic_cast<Hello *>(reply);
        EVT << "Neighbor LPI client has ID " << msg->getLpiClientID() << endl;
        neighborLPIClient = msg->getLpiClientID();
        simulation.getModule(neighborLPIClient)->subscribe("LPI_indication", this);
        delete reply;
    }
}

void LPI_client_100baseTX::receiveSignal(cComponent *src, simsignal_t signalId, const char* s) {
    Enter_Method_Silent();

    EVT << "LPIClient - Received signal: " << src->getFullPath() << "=" << s << endl;

    if ( simulation.getModuleByPath( src->getFullPath().c_str() )->getId() != neighborLPIClient )
        throw cRuntimeError("Not from my neighbour");

    if(strcmp(s,"AWAKE") == 0){
        EVT << "Received awake signal. Node is now awake\n";
        //hostAwake = true;
    }
    else if(strcmp(s,"partialAWAKE") == 0){
            EVT << "Now in partial LPI\n";
            //hostAwake = true;
            if(!partialLPI)
                throw cRuntimeError("Module params not consistent!");
            if(partialLPI){
                emit(powerSignal, pEeePowerIncrement - pEeeTransitionPower); // vado a 250 mW
            }

            emit(noLpi_powerSignal, noEeePowerIncrement);
    }
    else if(strcmp(s,"startLPI") == 0){
        EV << "My neighbor is going to LPI. Waiting to lower power consumption\n";
        EVT << "Received an LPI.request()" << endl;
        if(partialLPI)
            emit(powerSignal, -pEeeTransitionPower); // resto a 250 mW
        else
            emit(powerSignal, -eeeTransitionPower); // resto a 500 mW

    }
    else if(strcmp(s,"LPI") == 0){
        EV << "Now in LPI\n";
        emit(noLpi_powerSignal, -noEeePowerIncrement);
        if(partialLPI)
            emit(powerSignal, -pEeePowerIncrement + eeeTransitionPower); //se sono un normale nodo calo di 200 e torno a 250
        else
            emit(powerSignal, -eeePowerIncrement + eeeTransitionPower); // se non sono in LPI parz
    }
}

void LPI_client_100baseTX::receiveSignal(cComponent *src, simsignal_t signalId, long l)
{

    EVT << "LPIClient - Received signal: " << src->getFullPath() << "=" << l << endl;

    if (signalId == macSignals){
        switch(l){
            case START_LPI:
                EVT << "LPI.request() -> LPICLient received START_LPI" << endl;
                //scheduleAt(simTime() + sleepTime, startLpiTimer);
                EVT << "Triggering Low Power Idle\n";
                break;
            case QUIET_TX:
                EVT << "LPICLient state transition toward quiet." << endl;
                break;
        }
    }
    else
        throw cRuntimeError("Strange error happened");


    //EVT << " LpiSignal has listeners?" << hasListeners(lpiSignal);
}
