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


package eeepowerlink.deleted.nodes.inutili;

import eeepowerlink.src.LPI_client;
import eeepowerlink.src.ManagingNodeCli;
import eeepowerlink.src.ManagingNodeSrv;
import inet.linklayer.IEtherMAC;
import inet.linklayer.ethernet.EtherLLC;
import inet.linklayer.ethernet.EtherQoSQueue;


//
// An example host with one Ethernet port and a traffic generator that
// generates request-reply traffic directly over Ethernet. This host model
// does not contain higher layer protocols (IP, TCP). By default it is
// configured to use full-duplex MAC (no CSMA/CD).
//
module ManagingNode
{
    parameters:
        bool csmacdSupport = default(false);  // by default use CSMA/CD
        string macType = default(csmacdSupport ? "EtherMAC" : "EtherMACFullDuplex"); // ~EtherMAC or ~EtherMACFullDuplex
        string queueType = default(""); // DropTailQueue, DiffServQueue, or empty to use the internal queue
        @node();
        @labels(node,ethernet-node);
        @display("i=device/pc2");
    gates:
        inout ethg @labels(EtherFrame-conn);
    submodules:
        cli: ManagingNodeCli {
            parameters:
                @display("p=60,60");
                registerSAP = true;
        }
        srv: ManagingNodeSrv {
            parameters:
                registerSAP = true;
                @display("p=250,60");
        }
        llc: EtherLLC {
            parameters:
                @display("p=155,120");
            gates:
                upperLayerIn[2];
                upperLayerOut[2];
        }
        mac: <macType> like IEtherMAC {
            parameters:
                queueModule = "";
                @display("p=155,272;q=txQueue");
        }
        lpi_client: LPI_client {
            @display("p=155,201");
        }
    connections:

        cli.out --> llc.upperLayerIn[0];
        cli.in <-- llc.upperLayerOut[0];
        srv.out --> llc.upperLayerIn[1];
        srv.in <-- llc.upperLayerOut[1];

        mac.phys <--> ethg;
        lpi_client.upperLayerOut --> llc.lowerLayerIn;
        llc.lowerLayerOut --> lpi_client.upperLayerIn;
        lpi_client.lowerLayerOut --> mac.upperLayerIn;
        mac.upperLayerOut --> lpi_client.lowerLayerIn;
}
