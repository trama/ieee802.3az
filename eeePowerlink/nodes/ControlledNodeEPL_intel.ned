//
// Copyright (C) 2014 Federico Tramarin, Legnago, Italy
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


package eeepowerlink.nodes;

import eeepowerlink.nodes.IControlledNode;

import eeepowerlink.src.basetx100.EEE_EtherMACFullDuplex;
import eeepowerlink.src.basetx100.EPL_ControlledNode_intel;
import eeepowerlink.src.basetx100.LPI_client_100baseTX;
import inet.linklayer.ethernet.EtherEncap;
import inet.status.NodeStatus;

module ControlledNodeEPL_intel like IControlledNode
{
    parameters:
        bool hasStatus = default(false);
        bool csmacdSupport = default(false);  // by default use CSMA/CD
        //string macType = default(csmacdSupport ? "EtherMAC" : "EEE_EtherMACFullDuplex"); // ~EtherMAC or ~EtherMACFullDuplex
        @node();
        @labels(node,ethernet-node);
        @display("i=device/pc2;bgb=218,329");
        string queueType = default(""); // DropTailQueue, DiffServQueue, or empty to use the internal queue
    gates:
        inout ethg @labels(EtherFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=62,37;is=s");
        }
        llc: EtherEncap {
            parameters:
                @display("p=162,150");
        }
        mac: EEE_EtherMACFullDuplex {
            parameters:
                queueModule = (queueType == "" ? "" : "queue");
                @display("p=162,283;q=txQueue");
        }
        lpi_client: LPI_client_100baseTX {
            @display("p=62,283");
        }
        CN: EPL_ControlledNode_intel {
            @display("p=162,37");
        }
    connections:

        llc.lowerLayerOut --> mac.upperLayerIn;
        mac.upperLayerOut --> llc.lowerLayerIn;

        mac.phys <--> ethg;
        lpi_client.lpi2macControl --> mac.lpi2macControl;
        mac.mac2lpiControl --> lpi_client.mac2lpiControl;
        CN.out --> llc.upperLayerIn;
        llc.upperLayerOut --> CN.in;
}
