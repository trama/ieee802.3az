//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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


package eeepowerlink.deleted.src.inutili;

//
// Server side of the EtherCli model -- generates ~EtherAppResp packets
// with the number of bytes requested by the client in ~EtherAppReq.
// It should be connected directly to ~EtherLLC or a Ieee802NicXXX module.
//
simple ControlledNodeSrv
{
    parameters:
        double Ts @unit(s) = default(100us);
        double Tq @unit(s) = default(20ms);
        double Tw @unit(s) = default(30us);
        double TPowCycle @unit(s) = default(400us);  // Powerlink cycle period

        double Tplt @unit(s) = default(5us);
        double TPreq_Pres_CN @unit(s) = default(20us);  // delay between end of PReq reception and start of PRes transmission                 	
        double TPropag @unit(s) = default(20us);  // signal propagation time

        double DefNoEEEPower = default(0);
        double DefEEEPower = default(0);
        double HNoEEEPower = default(0);
        double LNoEEEPower = default(0);
        double HEEEPower = default(0);
        double LEEEPower = default(0);
        double L_TX_EEEPower = default(0);
        double L_RX_EEEPower = default(0);
        double L_TXRX_EEEPower = default(0);
        double L_RXTX_EEEPower = default(0);
        double IPower = default(0);
        bool onSoA = default(true); // se si mette falso allora si attiva la modalità idle dopo Pres

        double numHost = default(0);  // number of the Host
        volatile double sendInterval @unit(s) = default(uniform(0s,1s));  // interval between sending requests
        volatile int reqLength @unit(B) = default(1B);  // length of request packets
        volatile int respLength @unit(B) = default(1B);  // length of response packets
        bool registerSAP = default(false);   // whether to sent IEEE802CTRL_REGISTER_DSAP on startup
        @display("i=block/app");
        @signal[sentPk](type=cPacket);
        @signal[rcvdPk](type=cPacket);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,vector; interpolationmode=none);
    gates:
        input in @labels(Ieee802Ctrl/up);
        output out @labels(Ieee802Ctrl/down);
}
