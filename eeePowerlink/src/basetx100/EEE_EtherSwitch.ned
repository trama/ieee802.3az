//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//               2010 Zoltan Bojthe
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

import eeepowerlink.nodes.ISwitchEEE;
import inet.base.NotificationBoard;
import inet.networklayer.common.InterfaceTable;
import inet.status.NodeStatus;


//
// Model of an Ethernet switch.
//
// The duplexChannel attributes of the MACs must be set according to the
// medium connected to the port; if collisions are possible (it's a bus or hub)
// it must be set to false, otherwise it can be set to true.
// By default used half duples CSMA/CD mac
//
// This model does not contain the spanning tree algorithm.
//
module EEE_EtherSwitch like ISwitchEEE
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("i=device/switch");
        bool hasStatus = default(false);
        string addressTableFile = default("");
        bool csmacdSupport = default(false);  // by default use CSMA/CD
        //string macType = default(csmacdSupport ? "EtherMAC" : "EEE_EtherMACFullDuplex"); // ~EtherMAC or ~EtherMACFullDuplex
        //string relayUnitType = default("relayUnitForEEE"); // type of the ~IMACRelayUnit; currently possible
    gates:
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=70,50;is=s");
        }
        interfaceTable: InterfaceTable {
            @display("p=70,100;is=s");
        }
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=70,150;is=s");
        }
        relayUnit: relayUnitForEEE { //<relayUnitType> like IMACRelayUnit {
            parameters:
                addressTableFile = addressTableFile;
                @display("p=270,50");
        }
        eth[sizeof(ethg)]: EEE_100BASETX {
            parameters:
                mac.promiscuous = true;
                @display("p=207,150,row;q=txQueue");
        }
    connections:
        for i=0..sizeof(ethg)-1 {
            eth[i].upperLayerIn <-- relayUnit.ifOut++;
            eth[i].upperLayerOut --> relayUnit.ifIn++;
            eth[i].phys <--> ethg[i];
        }
}
