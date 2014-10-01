/*
 * Copyright (C) 2014 Federico Tramarin, Legnago, Italy
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

#ifndef __EPL_MN_INTEL_H
#define __EPL_MN_INTEL_H

#include "EPLDefs.h"
#include "MACAddress.h"
#include "EPLframe_m.h"

class EPL_ManagingNode_intel : public cSimpleModule, protected cListener {
    protected:

        int currentHost; // Host for AsyncSend
        bool persoPres; // Loss of PRes

        // Powerlink cycle parameters

        simtime_t eplCycleTime;
        simtime_t eplPreqPresTime;
        simtime_t propagationTime;
        simtime_t preqTimeOutTime;
        simtime_t wakeTime, partialWakeTime;
        simtime_t Ttx;
        simtime_t Tsi, Tsoc;
        simtime_t stopTime;
        simtime_t TimeAsync;
        simtime_t TimeEPL;
        simtime_t TPreq_Pres_MN;

        bool multiplexed;
        simtime_t muxEPLtime;
        std::vector<int> muxCN;
        std::vector< std::vector<int> > cycle_table;
        int numPollingPerCycle, numPolledSlave, cycle_count;

        int numberOfCN, numberOfSwitch, numberOfSubnetworks;
        int currentSubnet;
        int nodePerSubnet, midLevelNode;
        char tmp[10];

        int SoAHost; // Host for AsyncSend
        int cycleCount; // number of EPL cycles

        // send parameters
        long seqNum;
        int reqLength;
        bool debug;

        std::string logName() {
            return "EPL_ManagingNode";
        }

        MACAddress destMACAddress;

        bool hostAwake;
        bool lpiOnSoA;
        bool partialLPI;

        // self messages
        cMessage *preqTimer, *preqTimeoutTimer, *wakeUpTimer, *socTimer, *soaTimer;

        // receive statistics
        long packetsSent;
        long packetsReceived;
        simsignal_t sentPkSignal;
        simsignal_t rcvdPkSignal;
        simsignal_t rcvdPwSignal;
        simsignal_t rcvdPwNoEEESignal;
        simsignal_t powerSignal;
        simsignal_t noLpi_powerSignal;

    protected:
        virtual void initialize( int stage );
        virtual int numInitStages() const {
            return 2;
        }
        virtual void handleMessage( cMessage *msg );
        virtual void handleSelfMsg( cMessage *msg );
        virtual void finish();

        virtual MACAddress resolveDestMACAddress( int destAddress );
        virtual void sendPacket( int type, int dest );
        virtual void startIdle();
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, const char *s);
        virtual inline std::string nextMsgNameForPreq(int, MACAddress);
        virtual void read_cycle_table();
};

#endif
