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
/*
#include "EtherHubEEE.h"
#include "EPLframe_m.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Define_Module(EtherHubEEE);

enum EPLframeMessageKind {
        PREQ = 0,
        PRES = 1,
        SOA = 2,
        SOC = 3,
        ASYNC = 4,
        STOP_IDLE = 5,
        LPI_SIGNAL = 6
    };
enum HUBSelfMessageKind {
        START_LPI = 0,
        QUIET = 1,
        WAKE_UP = 2,
        WAKED_UP = 3,
        PLOT = 4,
        SEND_PRES = 5,
        START_POW = 6,
        SOA_GOLPI= 7

    };

int mpcur;
char name[30];
int nodesCycleHUB[17] = { 1, 2, 4, 1, 1, 1, 1, 1, 1, 1, 2, 2, 4, 4, 4, 4, 4 }; // MAC indexes of nodes

simsignal_t EtherHubEEE::pkSignal = SIMSIGNAL_NULL;
simsignal_t EtherHubEEE::rcvdPwSignal = SIMSIGNAL_NULL;
simsignal_t EtherHubEEE::rcvdPwNoEEESignal = SIMSIGNAL_NULL;

double percH;

static cEnvir& operator<<(cEnvir& out, cMessage *msg)
{
    out.printf("(%s)%s", msg->getClassName(), msg->getFullName());
    return out;
}

void EtherHubEEE::initialize()
{
    numPorts = gateSize("ethg");
    inputGateBaseId = gateBaseId("ethg$i");
    outputGateBaseId = gateBaseId("ethg$o");
    pkSignal = registerSignal("pk");
    rcvdPwSignal = registerSignal("rcvdPw");
    rcvdPwNoEEESignal = registerSignal("rcvdPwNoEEE");

    DefEEEPower  = par("DefEEEPower"); // LPI Power consumption with EEE
    DefNoEEEPower  = par("DefNoEEEPower"); // LPI Power consumption without EEE

    HNoEEEPower  = par("HNoEEEPower"); // ACTIVE Power consumption increment
    LNoEEEPower  = par("LNoEEEPower"); // ACTIVE Power consumption increment
    HEEEPower  = par("HEEEPower"); // ACTIVE Power consumption increment
    LEEEPower  = par("LEEEPower"); // LPI Power consumption decrement
    numHUB = par("numHUB"); // Number of the current HUB

    Ts = par("Ts"); // Time to go in LPI mode
    Tq = par("Tq"); // Refresh LPI signal period
    Tw = par("Tw"); // Wake up time
    TPowCycle = par("TPowCycle"); // EPL cycle time
    EV << "TEPL" << TPowCycle << "'\n";
    EV << "My number is HUB" << numHUB << "'\n";
    EV << "def " << DefEEEPower << "H " << HEEEPower << "L "<<LEEEPower;
    //printf("def %f \n",DefEEEPower);
    //printf("H %f \n",HEEEPower);
    //printf("L %f \n",LEEEPower);


    numMessages = 0;
    countGates = 0;
    WATCH(numMessages);

    IStartTime =  new simtime_t[numPorts];
    IEndTime =  new simtime_t[numPorts];

    gateLPI = new bool[numPorts];
    // ensure we receive frames when their first bits arrive
    for (int i = 0; i < numPorts; i++)
    {
        gate(inputGateBaseId + i)->setDeliverOnReceptionStart(true);
        gateLPI[i] = false; // il gate è in LPI
        IStartTime[i] = 0;
        IEndTime[i] = 0;

    }
    subscribe(POST_MODEL_CHANGE, this);  // we'll need to do the same for dynamically added gates as well

    checkConnections(true);

    timerMsg = new cMessage("startPow",START_POW);
    scheduleAt(simTime(), timerMsg);
}

void EtherHubEEE::checkConnections(bool errorWhenAsymmetric)
{
    int numActivePorts = 0;
    double datarate = 0.0;
    dataratesDiffer = false;

    for (int i = 0; i < numPorts; i++)
    {
        cGate *igate = gate(inputGateBaseId + i);
        cGate *ogate = gate(outputGateBaseId + i);
        if (!igate->isConnected() && !ogate->isConnected())
            continue;
        if (!igate->isConnected() || !ogate->isConnected())
        {
            // half connected gate
            if (errorWhenAsymmetric)
                throw cRuntimeError("The input or output gate not connected at port %i", i);
            dataratesDiffer = true;
            EV << "The input or output gate not connected at port " << i << ".\n";
            continue;
        }

        numActivePorts++;
        double drate = igate->getIncomingTransmissionChannel()->getNominalDatarate();

        if (numActivePorts == 1)
            datarate = drate;
        else if (datarate != drate)
        {
            if (errorWhenAsymmetric)
                throw cRuntimeError("The input datarate at port %i differs from datarates of previous ports", i);
            dataratesDiffer = true;
            EV << "The input datarate at port " << i << " differs from datarates of previous ports.\n";
        }

        cChannel *outTrChannel = ogate->getTransmissionChannel();
        drate = outTrChannel->getNominalDatarate();

        if (datarate != drate)
        {
            if (errorWhenAsymmetric)
                throw cRuntimeError("The output datarate at port %i differs from datarates of previous ports", i);
            dataratesDiffer = true;
            EV << "The output datarate at port " << i << " differs from datarates of previous ports.\n";
        }

        if (!outTrChannel->isSubscribed(POST_MODEL_CHANGE, this))
            outTrChannel->subscribe(POST_MODEL_CHANGE, this);
    }
}

void EtherHubEEE::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    Enter_Method_Silent();

    ASSERT(signalID == POST_MODEL_CHANGE);

    // if new gates have been added, we need to call setDeliverOnReceptionStart(true) on them
    cPostGateVectorResizeNotification *notif = dynamic_cast<cPostGateVectorResizeNotification*>(obj);
    if (notif)
    {
        if (strcmp(notif->gateName, "ethg") == 0)
        {
            int newSize = gateSize("ethg");
            for (int i = notif->oldSize; i < newSize; i++)
                gate(inputGateBaseId + i)->setDeliverOnReceptionStart(true);
        }
        return;
    }

    cPostPathCreateNotification *connNotif = dynamic_cast<cPostPathCreateNotification *>(obj);
    if (connNotif)
    {
        if ((this == connNotif->pathStartGate->getOwnerModule()) || (this == connNotif->pathEndGate->getOwnerModule()))
            checkConnections(false);
        return;
    }

    cPostPathCutNotification *cutNotif = dynamic_cast<cPostPathCutNotification *>(obj);
    if (cutNotif)
    {
        if ((this == cutNotif->pathStartGate->getOwnerModule()) || (this == cutNotif->pathEndGate->getOwnerModule()))
            checkConnections(false);
        return;
    }

    // note: we are subscribed to the channel object too
    cPostParameterChangeNotification *parNotif = dynamic_cast<cPostParameterChangeNotification *>(obj);
    if (parNotif)
    {
        cChannel *channel = dynamic_cast<cDatarateChannel *>(parNotif->par->getOwner());
        if (channel)
        {
            cGate *gate = channel->getSourceGate();
            if (gate->pathContains(this))
                checkConnections(false);
        }
        return;
    }
}

void EtherHubEEE::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if(msg->getKind() == START_POW)
            {
                for (int i = 0; i < numPorts; i++)
                {
                    power = DefEEEPower;
                    emit(rcvdPwSignal, power);
                    power = DefNoEEEPower;
                    emit(rcvdPwNoEEESignal, power);
                    //printf("incremento %f \n",DefEEEPower);
                }

            }
    }
    else
    {
        if (dataratesDiffer)
            checkConnections(true);

        // Handle frame sent down from the network entity: send out on every other port
        int arrivalPort = msg->getArrivalGate()->getIndex();
        EV << "Frame " << msg << " arrived on port " << arrivalPort << ", broadcasting on all other ports\n";

        if( strcmp(msg->getName(),"LPISignal") == 0 && gateLPI[arrivalPort])  // LPI received
        {
            //EV<< "LPI signal on port " << arrivalPort << "go LPI mode \n";
            power = LEEEPower;
            //EV << "Quiet, decremento potenza "<< power << "\n";
            emit(rcvdPwSignal, power);

            power = LNoEEEPower;
            //EV << "Quiet, decremento potenza "<< power << "\n";
            emit(rcvdPwNoEEESignal, power);
            EV << "Arrivato LPISignal da " << arrivalPort << "decremento potenza di \n" << LEEEPower;
            //printf("HUB%d-%d entra dall'LPI \n",numHUB,arrivalPort);

            //printf("porta %d,decremento %f \n",arrivalPort,LEEEPower);
            gateLPI[arrivalPort] = false; // disattivo porta arrivalPort
            IStartTime[arrivalPort] = simTime() + Ts;
            countGates++;
            //printf("numero di LPI arrivati %d, numero di porte%d ",countGates,numPorts);
            if(countGates == numPorts-1)
            {
                for (int i = 0; i < numPorts; i++)
                {
                    if (gateLPI[i])
                    {
                        cGate *ogate = gate(outputGateBaseId + i);
                        if (!ogate->isConnected())
                            continue;
                        cMessage *msg2 = msg->dup();
                        // stop current transmission
                        ogate->getTransmissionChannel()->forceTransmissionFinishTime(SIMTIME_ZERO);
                        // send
                        send(msg2, ogate);

                        power = LEEEPower;
                        //EV << "Quiet, decremento potenza "<< power << "\n";
                        emit(rcvdPwSignal, power);

                        power = LNoEEEPower;
                        //EV << "Quiet, decremento potenza "<< power << "\n";
                        emit(rcvdPwNoEEESignal, power);
                        //printf("HUB%d-%d entra dall'LPI \n",numHUB,i);
                        EV << "Ultima porta " << i << "decremento potenza di \n" << LEEEPower;
                        gateLPI[i] = false; // disattivo porta arrivalPort
                        IStartTime[i] = simTime() + Ts;
                        //printf("porta %d, decremento %f \n",i,LEEEPower);
                    }
                }
            }
        }
        else
        {
            mpcur = atoi(&(msg->getName()[8]));
            sprintf(name, "StopIDLE%d",mpcur);
            if( strcmp(msg->getName(),name) == 0 && mpcur%nodesCycleHUB[numHUB-1] == 0)
            {
                EV << "ENTRATO";
                countGates = 0;
                for (int i = 0; i < numPorts; i++)
                {
                    if( !gateLPI[i])
                    {
                        gateLPI[i] = true; // il gate non è più in LPI
                        IEndTime[i] = simTime();
                        LPITime = IEndTime[i] -IStartTime[i];
                        percH = LPITime/(nodesCycleHUB[numHUB-1]*TPowCycle);
                        power = HEEEPower;
                        //printf("HUB%d-%d esce dall'LPI \n",numHUB,i);
                        EV << "Tempo trascorso in LPI dall'Hub " <<  numHUB << "port" << i << "Tidle = " << LPITime << "\n";
                        printf("Percentuale LPI dall'hub%d-%d: %f us\n",numHUB,i, percH);
                        //EV << "ACTIVE, incremento potenza "<< power << "\n";
                        emit(rcvdPwSignal, power);

                        power = HNoEEEPower;
                        //EV << "ACTIVE, incremento potenza "<< power << "\n";
                        emit(rcvdPwNoEEESignal, power);
                        //printf("incremento %f \n",HEEEPower);
                    }
                }
                numMessages++;
                emit(pkSignal, msg);

                if (numPorts <= 1)
                {
                    delete msg;
                    return;
                }

                for (int i = 0; i < numPorts; i++)
                {
                    if (i != arrivalPort)
                    {
                        cGate *ogate = gate(outputGateBaseId + i);
                        if (!ogate->isConnected())
                            continue;

                        bool isLast = (arrivalPort == numPorts-1) ? (i == numPorts-2) : (i == numPorts-1);
                        cMessage *msg2 = isLast ? msg : msg->dup();

                        // stop current transmission
                        ogate->getTransmissionChannel()->forceTransmissionFinishTime(SIMTIME_ZERO);

                        // send
                        send(msg2, ogate);

                        if (isLast)
                            msg = NULL;  // msg sent, do not delete it.
                    }
                }
                delete msg;

            }
            else if( strcmp(msg->getName(),name) != 0)
            {
                numMessages++;
                emit(pkSignal, msg);

                if (numPorts <= 1)
                {
                    delete msg;
                    return;
                }

                for (int i = 0; i < numPorts; i++)
                {
                    if (i != arrivalPort)
                    {
                        cGate *ogate = gate(outputGateBaseId + i);
                        if (!ogate->isConnected())
                            continue;

                        bool isLast = (arrivalPort == numPorts-1) ? (i == numPorts-2) : (i == numPorts-1);
                        cMessage *msg2 = isLast ? msg : msg->dup();

                        // stop current transmission
                        ogate->getTransmissionChannel()->forceTransmissionFinishTime(SIMTIME_ZERO);

                        // send
                        send(msg2, ogate);

                        if (isLast)
                            msg = NULL;  // msg sent, do not delete it.
                    }
                }
                delete msg;
            }
        }
    }
}


void EtherHubEEE::finish()
{
    simtime_t t = simTime();
    recordScalar("simulated time", t);

    if (t > 0)
        recordScalar("messages/sec", numMessages / t);
}

*/
