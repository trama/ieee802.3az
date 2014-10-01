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

#include "LPIclient.h"
#include "powerListener.h"
#include <cassert>

Define_Module(LPI_client);

void LPI_client::initialize( int stage ) {
    if ( stage == 1 ) {

        powerSignal = registerSignal("powerSignal");
        lpiSignal = registerSignal("LPI_indication");
        lpi_time = registerSignal("lpi_time");
        partial_lpi_time = registerSignal("partial_lpi_time");
        thisMN = par("iAmTheMn").boolValue();
        powerListener *listener = new powerListener(true);
        subscribe(powerSignal, listener);
        powerListener *listener2 = new powerListener(false);
        noLpi_powerSignal = registerSignal("noLpi_powerSignal");
        subscribe(noLpi_powerSignal, listener2);

        debug = par("debug").boolValue();
        partialLPI = par("partialLPI").boolValue();
        upperLayerIn = findGate("upperLayerIn");
        upperLayerOut = findGate("upperLayerOut");
        upperControlIn = findGate("upperControlIn");
        upperControlOut = findGate("upperControlOut");
        lowerLayerIn = findGate("lowerLayerIn");
        lowerLayerOut = findGate("lowerLayerOut");

        sleepTime = par("sleepTime"); // Time to go in LPI mode
        quietTime = par("quietTime");
        wakeTime = par("wakeTime");
        partialWakeTime = par("partialWakeTime");
        partialSleepTime = par("partialSleepTime");
        refreshTime = par("refreshTime");

        startLpiTimer = new cMessage("startLPI", START_LPI);
        quietTimer = new cMessage("refreshCycle", QUIET);
        refreshTimer = new cMessage("refreshCycle", REFRESH);
        wakeUpTimer = new cMessage("wakeUp", WAKE_UP);
        wakedUpTimer = new cMessage("awake", WAKED_UP);

        totPower = par("powerConsumption"); // Power consumption when active
        lpiPower = par("powerConsumptionEEE"); // LPI Power consumption with EEE
        pLpiPower = par("powerConsumptionPEEE"); // LPI Power consumption with partial EEE
        eeePowerIncrement = par("eeePowerIncrement");
        noEeePowerIncrement = par("noEeePowerIncrement");
        eeeTransitionPower = par("eeeTransitionPower");
        pEeePowerIncrement = par("pEeePowerIncrement");
        pEeeTransitionPower = par("pEeeTransitionPower");

        queue = new cQueue("Lpi packet queue");

        emit(powerSignal, lpiPower); // save default power in LPI
        emit(noLpi_powerSignal, totPower-noEeePowerIncrement);
    }
}

void LPI_client::handleSelfMsg(cMessage *msg) {
	switch (msg->getKind()) {
		case START_LPI:
	        active = false;
			EVT << "START LPI - go LPI " << simTime() << "\n";
			emit(noLpi_powerSignal, -noEeePowerIncrement);
			if(partialLPI && !thisMN)
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
            EVT << "Sending periodic LPI Refresh Signal!\n"<<endl;
            scheduleAt(simTime() + refreshTime, refreshTimer);
            if(partialLPI && !thisMN)
                emit(powerSignal, pEeePowerIncrement - eeeTransitionPower); //se sono un normale nodo calo di 200 e torno a 250
            else
                emit(powerSignal, eeePowerIncrement - eeeTransitionPower); // se non sono in LPI parz
                                    // oppure sono il master calo di 450, e torno a 50 mW
            EVT << "Rescheduling periodic LPI Refresh Signal!\n"<<endl;
            scheduleAt(simTime() + quietTime, quietTimer);
            EV << "Quiet\n";
            break;
		case REFRESH:
		    if(partialLPI && !thisMN)
                emit(powerSignal, -pEeePowerIncrement + eeeTransitionPower); //se sono un normale nodo calo di 200 e torno a 250
            else
                emit(powerSignal, -eeePowerIncrement + eeeTransitionPower); // se non sono in LPI parz
                                    // oppure sono il master calo di 450, e torno a 50 mW

		    break;
		case WAKE_UP:
		    EV << "Exiting Low Power Idle. Waking up\n";
			IEndTime = simTime();

			if(1){
                cMessage *req = (cMessage *) msg->getContextPointer();
                bool verifyPLPI = (check_and_cast<EPLframe*>(req) )->getPartialLPI();
                EV << "Context Pointer says that we should activate partial LPI? " <<
                        verifyPLPI << endl;

                if(partialLPI != verifyPLPI)
                    throw cRuntimeError("Module parameters are not consistent!");

                //delete req;
			}
            if(!thisMN && partialLPI){
                emit(powerSignal, pEeeTransitionPower); // resto a 50 mW
                PIStartTime = simTime();
            }
            else{
                EV << "I am the MN so I does not use partial LPI" << endl;
                emit(powerSignal, eeeTransitionPower); // resto a 50 mW
            }
            emit(lpi_time, IEndTime - IStartTime);
            //emit(lpi_time, IEndTime - PIStartTime);
			//wakedUpTimer->setContextPointer(msg->getContextPointer());
			scheduleAt(simTime() + wakeTime, wakedUpTimer);

			break;
		case WAKED_UP:
            active = true;
			EV << "STOP LPI -- Tempo trascorso in LPI Tidle = " << (IEndTime - IStartTime) << endl;

			if(!queue->empty()){
                if(queue->length()>1)
                    throw cRuntimeError("LpiClient Queue length is wrong");
                cMessage *tmp = check_and_cast<cMessage *>( queue->pop() );
                sendDown( tmp );
            }

			if(!thisMN && partialLPI){
			    emit(lpiSignal, "partialAWAKE");
			    emit(powerSignal, pEeePowerIncrement - pEeeTransitionPower); // vado a 250 mW
			}
			else{
			    emit(lpiSignal, "AWAKE");
			    emit(powerSignal, eeePowerIncrement - eeeTransitionPower); // vado a 500 mW
			}
			emit(noLpi_powerSignal, noEeePowerIncrement);
			break;
		case START_LPITX:
		    EV << "returning in partial LPI" << endl;
	        if(!thisMN && partialLPI){
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
            if(!thisMN && partialLPI){
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

void LPI_client::handleLowerMsg(cMessage *msg) {

	EPLframe *req =  dynamic_cast<EPLframe *>(msg); // check if EPL frame received

	if(!req){
	    EVT << "Msg is not an EPL frame. " << endl;
	    throw cRuntimeError("Not an EPL frame");
	}

	if (req->getType() == STOP_IDLE) {
		EVT << " Received an LPI.indication()" <<
		        (req->getPartialLPI() ? " with partial LPI" : "") << endl;
		EVT << " Inform upper layer" << endl;
		wakeUp(req);
		sendCtrlUp(req->dup());
	}
	else{
	    EVT << " Received msg from down. Forwarding Up." << endl;
	    sendUp(msg);
	}
}

void LPI_client::handleUpperCtrl(cMessage *msg){
    EVT << "msg " << msg->getClassName() << msg->getFullPath() << endl;

    EPLframe *req = check_and_cast<EPLframe *>(msg); // EPL frame received

    if (req->getType() == LPI_SIGNAL) {
        EVT << " Received an LPI.request()" << endl;
        EVT << " Sending request down" << endl;
        sendDown(req);
        if(partialLPI)
            emit(powerSignal, -pEeeTransitionPower); // resto a 250 mW
        else
            emit(powerSignal, -eeeTransitionPower); // resto a 500 mW

        scheduleAt(simTime() + sleepTime, startLpiTimer);
        EV << "Triggering Low Power Idle\n";
    }
    else if(req->getType() == EXIT_PLPI) {
        EVT << "We must totally wake Up from partial LPI." << endl;
        fullActive(req);
        cMessage *tmp =  req->dup(); //duplica il messaggio
        tmp->setControlInfo(msg->removeControlInfo()); //bisogna attaccarci la controlInfo
        queue->insert(tmp);
        EVT << "Queue length " << queue->length() << endl;
    }
    else if(req->getType() == RETURN_PLPI) {
        EVT << "Return to partial LPI." << endl;
        EVT << "Received an LPI.request()" << endl;
        EV << "Triggering partial Low Power Idle\n";
        cMessage *tmp = new cMessage("returnLPI", START_LPITX);
        scheduleAt(simTime() + partialSleepTime, tmp);
    }
    else if(req->getType() == STOP_IDLE) { // arrive stop idle from MN
        EVT << "We must wake Up, and forwarding down the StopIDLE." << endl;
        wakeUp(req);
        cMessage *tmp =  req->dup(); //duplica il messaggio
        tmp->setControlInfo(msg->removeControlInfo()); //bisogna attaccarci la controlInfo
        queue->insert(tmp);
        EVT << "Queue length " << queue->length() << endl;
    }
}

void LPI_client::handleUpperMsg(cMessage *msg) {
    EVT << " Received msg from Up. Forwarding Down." << endl;
    sendDown(msg);
}

void LPI_client::wakeUp(EPLframe *frame) {
	if(quietTimer->isScheduled())
		cancelEvent(quietTimer);

	if(wakeUpTimer->getContextPointer() == NULL)
	    wakeUpTimer->setContextPointer(frame);
	scheduleAt(simTime(), wakeUpTimer);
}

void LPI_client::fullActive(EPLframe *frame) {
    cMessage *timer = new cMessage("FullActivation", FULL_ACTIVE);
    if(timer->getContextPointer() == NULL)
        timer->setContextPointer(frame);
    scheduleAt(simTime() + partialWakeTime, timer);
}

void LPI_client::finish() {
    cancelAndDelete(quietTimer);
    cancelAndDelete(startLpiTimer);
    cancelAndDelete(wakeUpTimer);
    cancelAndDelete(wakedUpTimer);
    if(!queue->empty())
        queue->clear();
    queue->~cQueue();

}

void LPI_client::sendDown(cMessage *msg) {
	send(msg, lowerLayerOut);
}

void LPI_client::sendUp(cMessage *msg) {
	send(msg, upperLayerOut);
}

void LPI_client::sendCtrlUp(cMessage *msg) {
   send(msg, upperControlOut);
}

