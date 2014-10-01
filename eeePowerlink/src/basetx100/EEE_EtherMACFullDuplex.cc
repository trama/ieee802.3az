//
// Copyright (C) 2014 Federico Tramarin
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "EEE_EtherMACFullDuplex.h"

#include "EtherFrame_m.h"
#include "IPassiveQueue.h"
#include "NotificationBoard.h"
#include "NotifierConsts.h"
#include "InterfaceEntry.h"
#include "hello_m.h"

Define_Module(EEE_EtherMACFullDuplex);

EEE_EtherMACFullDuplex::EEE_EtherMACFullDuplex(){}

void EEE_EtherMACFullDuplex::initialize(int stage)
{

    if (stage == 0)
    {
        debug = par("debug").boolValue();
        eee = par("eee_enabled").boolValue();
        EVT << "IEEE802.3az amendment is simulated!\n" << endl;
        macSignals = registerSignal("MAC-LPIClient signals");
        //subscribe(macSignals, this);
        wakeUpTimer = new cMessage("wakeUp", WAKE_UP);
        lpiState = ACTIVE_TX;
        mac2lpiControl = findGate("mac2lpiControl");
        lpi2macControl = findGate("lpi2macControl");
    }
    else if(stage==1)
        beginSendFrames();//emit(macSignals, START_LPI);
    EtherMACBase::initialize(stage);
}

void EEE_EtherMACFullDuplex::initializeStatistics()
{
    EtherMACFullDuplex::initializeStatistics();
}

void EEE_EtherMACFullDuplex::initializeFlags()
{
    EtherMACFullDuplex::initializeFlags();
}

void EEE_EtherMACFullDuplex::finish()
{
    cancelAndDelete(wakeUpTimer);
    EtherMACFullDuplex::finish();
}

void EEE_EtherMACFullDuplex::handleMessage(cMessage *msg){
    if (msg->getArrivalGateId() == lpi2macControl)
        handleLpi2MacControl(msg);
    else if (msg->getArrivalGateId() == physInGate->getId() && (msg->getKind()==12345 || msg->getKind()==12346) ){
        EVT << "Arrived a discovery request. Msg " << msg->getFullName() << endl;
        Hello *tmp;
        switch (msg->getKind()){
        case 12345:
            tmp = dynamic_cast<Hello *>(msg);
            EVT << "A neighbor sent me an HELLO message. Replying." << endl;
            tmp->setName("HELLO-REPLY");
            tmp->setKind(12346);
            tmp->setLpiClientID( gate(mac2lpiControl)->getNextGate()->getOwnerModule()->getId() );
            send(msg,physOutGate);
            break;
        case 12346:
            EVT << "A neighbor replied to my request with a " << msg->getName() << " Passing to my LPI client." << endl;
            send(msg, mac2lpiControl);
            break;
        }

    }
    else EtherMACFullDuplex::handleMessage(msg);
}

void EEE_EtherMACFullDuplex::handleSelfMessage(cMessage *msg){
    EVT << "Received self message " << msg << endl;
    if(msg == wakeUpTimer)
        handleLinkWakedUp();
    else
        EtherMACFullDuplex::handleSelfMessage(msg);
}

void EEE_EtherMACFullDuplex::handleLpi2MacControl(cMessage *msg){
    if(msg->getKind() == WAKED_UP){
        delete msg;
        handleLinkWakedUp();
    }
    else if(msg->getKind() == 12345){
        EVT << "Received request to find my neighbour.\n"
                "Remember, this is only a LOGIC operation."<< endl;
        send(msg, physOutGate);
    }

}

void EEE_EtherMACFullDuplex::processFrameFromUpperLayer(EtherFrame *frame)
{
    if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
        frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);  // "padding"

    frame->setFrameByteLength(frame->getByteLength());

    EVT << "Received frame from upper layer: " << frame << endl;

    emit(packetReceivedFromUpperSignal, frame);

    if (frame->getDest().equals(address))
    {
        error("logic error: frame %s from higher layer has local MAC address as dest (%s)",
                frame->getFullName(), frame->getDest().str().c_str());
    }

    if (frame->getByteLength() > MAX_ETHERNET_FRAME_BYTES)
    {
        error("packet from higher layer (%d bytes) exceeds maximum Ethernet frame size (%d)",
                (int)(frame->getByteLength()), MAX_ETHERNET_FRAME_BYTES);
    }

    if (!connected || disabled)
    {
        EVT << (!connected ? "Interface is not connected" : "MAC is disabled") << " -- dropping packet " << frame << endl;
        emit(dropPkFromHLIfaceDownSignal, frame);
        numDroppedPkFromHLIfaceDown++;
        delete frame;

        requestNextFrameFromExtQueue();
        return;
    }

    // fill in src address if not set
    if (frame->getSrc().isUnspecified())
        frame->setSrc(address);

    bool isPauseFrame = (dynamic_cast<EtherPauseFrame*>(frame) != NULL);

    if (!isPauseFrame)
    {
        numFramesFromHL++;
        emit(rxPkFromHLSignal, frame);
    }

    if (txQueue.extQueue)
    {
        ASSERT(curTxFrame == NULL);
        curTxFrame = frame;
    }
    else
    {
        if (txQueue.innerQueue->isFull())
            error("txQueue length exceeds %d -- this is probably due to "
                  "a bogus app model generating excessive traffic "
                  "(or if this is normal, increase txQueueLimit!)",
                  txQueue.innerQueue->getQueueLimit());
        // store frame and possibly begin transmitting
        EVT << "Frame " << frame << " arrived from higher layers, enqueueing\n";
        txQueue.innerQueue->insertFrame(frame);

        if (!curTxFrame && !txQueue.innerQueue->empty())
            curTxFrame = (EtherFrame*)txQueue.innerQueue->pop();
    }

    if (transmitState == TX_IDLE_STATE){
        EVT << "Ethernet-MAC is in TX_IDLE_STATE and is QUIET.\n"<<
                "At least 1 pkt in the buffer, we must wake-up!\n";
        scheduleLpiWakeUp();
    }
}

void EEE_EtherMACFullDuplex::scheduleLpiWakeUp(){
    lpiState = TRANSITION_TX;
    emit(macSignals, TRANSITION_TX);
    //simtime_t endWakeUpTime = simTime() + wakeTime;
    //scheduleAt(endWakeUpTime, wakeUpTimer);
    cMessage *mm = new cMessage("TX attivati", WAKE_UP);
    send(mm, mac2lpiControl);
}

void EEE_EtherMACFullDuplex::handleLinkWakedUp(){
    if (transmitState == TX_IDLE_STATE && lpiState == TRANSITION_TX){
        lpiState = ACTIVE_TX;
        EVT << "Ethernet-MAC is in TX_IDLE_STATE and is now ACTIVE. Start transmitting"<<endl;
        startFrameTransmission();
    }
    else
        throw cRuntimeError("We should transmit, but wrong states were detected");
}

void EEE_EtherMACFullDuplex::beginSendFrames()
{
    if (curTxFrame)
    {
        // Other frames are queued, transmit next frame
        EV << "Transmit next frame in output queue\n";
        startFrameTransmission();
    }
    else
    {
        // No more frames
        // if we are here and EEE is enabled, then we should start the wake procedure
        transmitState = TX_IDLE_STATE;
        if (!txQueue.extQueue){
            // Output only for internal queue (we cannot be shure that there
            //are no other frames in external queue)
            EV << "No more frames to send, transmitter set to idle\n";
        }
        // TODO: signal wake start
        emit(macSignals, QUIET_TX); // test signals, one day these will substitute messages.
        // triggering lpi to quiet
        cMessage *mm = new cMessage("TX go quiet", START_LPI);
        if (simTime() == 0)
            sendDelayed(mm, 1e-6, mac2lpiControl);
        else
            send(mm, mac2lpiControl);
    }
}

void EEE_EtherMACFullDuplex::receiveSignal(cComponent *src, simsignal_t signalId, long l)
{
    //Enter_Method_Silent();
    ASSERT(signalId == macSignals);
    EVT << "Received a signal " << getSignalName(signalId) << " with value " << l << endl;
}

