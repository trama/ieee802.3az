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

#ifndef __INET_ETHERAPPCLI_H
#define __INET_ETHERAPPCLI_H

#include "INETDefs.h"

#include "MACAddress.h"
#include "EPLframe_m.h"




/**
 * Simple traffic generator for the Ethernet model.
 */
class ManagingNodeCli_noEEE : public cSimpleModule
{
    enum MNNumbers {
        //N = 14
        //N = 10
        //N = 13
        N = 3
    };
    enum MNSelfMessageKind {
        WAKE_UP = 0,
        SEND_SOC = 1,
        POLLING = 2,
        SEND_SOA = 3
    };
    enum EPLframeMessageKind {
            PREQ = 0,
            PRES = 1,
            SOA = 2,
            SOC = 3,
            ASYNC = 4,
            STOP_IDLE = 5,
            LPI_SIGNAL = 6
        };
  protected:


    // Powerlink cycle parameters

    simtime_t TPowCycle;
    simtime_t TPreq_Pres_CN;
    simtime_t TPropag;
    simtime_t TwaitPres;
    simtime_t Tw;
    simtime_t d;
    simtime_t TCN;

    simtime_t TimeAsync;
    simtime_t TimeEPL;
    simtime_t TimePOLL;
    int numHost;
    char nodes[N][7]; // MAC indexes of nodes
    char tmp[10];
    int SoAHost; // Host for AsyncSend
    int cycleCount; // number of EPL cycles
    // send parameters
    long seqNum;
    cPar *reqLength;
    cPar *respLength;
    cPar *sendInterval;

    int localSAP;
    int remoteSAP;
    int kind;
    bool timerEnd;
    MACAddress destMACAddress;


    // self messages
    cMessage *timerMsg;
    simtime_t stopTime;

    // receive statistics
    long packetsSent;
    long packetsReceived;
    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;
    static simsignal_t rcvdPwSignal;
    static simsignal_t rcvdPwNoEEESignal;


    cOutVector powerVector;
    cOutVector powerNoEEEVector;


  public:

    ManagingNodeCli_noEEE();
    ~ManagingNodeCli_noEEE();

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const {return 2;}
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    virtual MACAddress resolveDestMACAddress(char *destAddress);
    virtual void sendPacket(int type, int dest);
    virtual void receivePacket(cPacket *msg);
    virtual void registerDSAP(int dsap);
};

#endif
