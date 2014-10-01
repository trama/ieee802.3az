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

#ifndef __INET_ETHERAPPSRV_H
#define __INET_ETHERAPPSRV_H

#include "INETDefs.h"

#include "MACAddress.h"
#include "EPLframe_m.h"
#define MAX_REPLY_CHUNK_SIZE   1497


/**
 * Server-side process EtherAppCli.
 */
class ControlledNodeSrv : public cSimpleModule
{

    private:
    double power;
    //cOutVector powerVector;

    enum MNNumbers {
        N = 13
    };

    double perc; // LPI time

    bool onSoa; // true : LPI after SoA, false : LPI after PRes
    bool wakedUp; // true : current node already in active mode, no need to wake up!
    double zero;

enum CNSelfMessageKind {
        START_LPI = 0,
        QUIET = 1,
        WAKE_UP = 2,
        WAKED_UP = 3,
        PLOT = 4,
        SEND_PRES = 5,
        START_POW = 6,
        SOA_GOLPI= 7,
        START_LPITX= 8,
        WAKE_AFTER = 9
    };

    protected:
    int localSAP;
    int remoteSAP;
    char SoAString[30];
    int numHost;
    double DefEEEPower;
    double DefNoEEEPower;
    double HNoEEEPower;
    double LNoEEEPower;
    double IPower;
    double HEEEPower;
    double LEEEPower;
    double L_TX_EEEPower;
    double L_RX_EEEPower;
    double L_TXRX_EEEPower;
    double L_RXTX_EEEPower;


    long seqNum;
    cPar *reqLength;
    cPar *respLength;
    cPar *sendInterval;

    MACAddress destMACAddress;
    MACAddress srcAddr;

    // self messages
    cMessage *timerMsg, *sendPresTimer, *startLpiTimer, *soaGoLpiTimer, *wakeAfterTimer,
                *startLpiTxTimer, *wakeUpTimer, *wakedUpTimer, *quietTimer, *plotTimer;
    simtime_t TEpl;
    simtime_t stopTime;
    simtime_t Ts;
    simtime_t Tq;
    simtime_t Tw;
    simtime_t TPowCycle;
    simtime_t IStartTime;
    simtime_t IEndTime;
    simtime_t Tplt;
    simtime_t TPreq_Pres_CN;
    simtime_t TPropag;
    simtime_t nextWake;

    // statistics
    long packetsSent;
    long packetsReceived;
    simsignal_t sentPkSignal;
    simsignal_t rcvdPkSignal;
    simsignal_t rcvdPwSignal;
    simsignal_t rcvdPwNoEEESignal;
    simsignal_t rcvdPw_1Signal;
    simsignal_t rcvdPw_2Signal;


    EPLframe *datapacket;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void handleSelfMsg(cMessage *msg);
    virtual void finish();
    void goLPI();
    void goLPITX();
    void wakeUp();
    void registerDSAP(int dsap);
    void sendLPISignal();
    void sendPacket(cPacket *datapacket, const MACAddress& destAddr);
};

#endif
