//
// Copyright (C) 2006 Levente Meszaros
// Copyright (C) 2010 Zoltan Bojthe
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


package eeepowerlink.src.basetx100;

import inet.linklayer.IEtherMAC;

simple EEE_EtherMACFullDuplex like IEtherMAC
{
    parameters:
        bool debug = default(false);
        bool eee_enabled = default(true);   // if true, EEE is enabled
        bool promiscuous = default(false);  // if true, all packets are received, otherwise only the
                                            // ones with matching destination MAC address
        string address = default("auto");   // MAC address as hex string (12 hex digits), or
                                            // "auto". "auto" values will be replaced by
                                            // a generated MAC address in init stage 0.
        bool duplexMode = default(true);    // must be set to "true", as EtherMACFullDuplex does not support half-duplex operation
                                            // (parameter is present to reduce the risk of accidental misconfiguration)
        int txQueueLimit = default(1000);   // maximum number of frames queued up for transmission in the internal queue
                                            // (only used if queueModule==""); additional frames cause a runtime error
        string queueModule = default("");   // name of optional external queue module
        int mtu @unit("B") = default(1500B);
        @display("i=block/rxtx");

        @signal[txPk](type=EtherFrame);
        @signal[rxPkOk](type=EtherFrame);
        @signal[txPausePkUnits](type=long);
        @signal[rxPausePkUnits](type=long);
        @signal[rxPkFromHL](type=EtherFrame);
        @signal[dropPkNotForUs](type=EtherFrame);
        @signal[dropPkBitError](type=EtherFrame);
        @signal[dropPkIfaceDown](type=EtherTraffic);        // emitted at end of receiving
        @signal[dropPkFromHLIfaceDown](type=EtherFrame);
        @signal[packetSentToLower](type=EtherFrame);
        @signal[packetReceivedFromLower](type=EtherFrame);
        @signal[packetSentToUpper](type=EtherFrame);
        @signal[packetReceivedFromUpper](type=EtherFrame);

        @statistic[txPk](title="packets transmitted"; source=txPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rxPkOk](title="packets received OK"; source=rxPkOk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[passedUpPk](title="packets passed to higher layer"; source=packetSentToUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[txPausePkUnits](title="pause units sent"; record=count,sum,vector; interpolationmode=none);
        @statistic[rxPausePkUnits](title="pause units received"; record=count,sum,vector; interpolationmode=none);
        @statistic[rxPkFromHL](title="packet bytes from higher layer"; source=rxPkFromHL; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[droppedPkIfaceDown](title="packets dropped/interface down"; source=dropPkIfaceDown; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[droppedPkBitError](title="packets dropped/bit error"; source=dropPkBitError; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[droppedPkNotForUs](title="packets dropped/not for us"; source=dropPkNotForUs; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);

    gates:
        input upperLayerIn @labels(EtherFrame);    // to ~EtherLLC or ~EtherEncap or ~MACRelayUnitPP
        output upperLayerOut @labels(EtherFrame);  // to ~EtherLLC or ~EtherEncap or ~MACRelayUnitPP
        inout phys @labels(EtherFrame);            // to physical layer or the network
        input lpi2macControl @labels(LPI);    // from ~Ethernet MAC
        output mac2lpiControl @labels(LPI);  // to ~Ethernet MAC
}

//
// TODO documentation
//
module EEE_100BASETX
{
    gates:
        input upperLayerIn @labels(EtherFrame);    // to ~EtherLLC or ~EtherEncap or ~MACRelayUnitPP
        output upperLayerOut @labels(EtherFrame);  // to ~EtherLLC or ~EtherEncap or ~MACRelayUnitPP
        inout phys @labels(EtherFrame);            // to physical layer or the network
    submodules:
        mac: EEE_EtherMACFullDuplex {
            @display("p=256,106");
        }
        lpi_client: LPI_client_100baseTX {
            @display("p=89,106");
        }
    connections:
        lpi_client.lpi2macControl --> mac.lpi2macControl;
        mac.mac2lpiControl --> lpi_client.mac2lpiControl;
        mac.phys <--> phys;
        mac.upperLayerOut --> upperLayerOut;
        upperLayerIn --> mac.upperLayerIn;
}

