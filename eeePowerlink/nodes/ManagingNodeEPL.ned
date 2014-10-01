
package eeepowerlink.nodes;

import eeepowerlink.nodes.IManagingNode;
import eeepowerlink.src.EPL_ManagingNode;
import eeepowerlink.src.LPI_client;
import inet.linklayer.IEtherMAC;
import inet.linklayer.ethernet.EtherEncap;


module ManagingNodeEPL like IManagingNode
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
        MN: EPL_ManagingNode {
            parameters:
                @display("p=155,58");
                registerSAP = false;
        }
        llc: EtherEncap {
            parameters:
                @display("p=69,272");
            //gates:
            //    upperLayerIn[1];
            //    upperLayerOut[1];
        }
        mac: <macType> like IEtherMAC {
            parameters:
                queueModule = "";
                @display("p=155,272;q=txQueue");
        }
        lpi_client: LPI_client {
            @display("p=155,173");
        }
    connections:

        mac.phys <--> ethg;
        llc.lowerLayerOut --> mac.upperLayerIn;
        mac.upperLayerOut --> llc.lowerLayerIn;
        llc.upperLayerOut --> lpi_client.lowerLayerIn;
        lpi_client.lowerLayerOut --> llc.upperLayerIn;
        MN.ctrlOut --> lpi_client.upperControlIn;
        lpi_client.upperControlOut --> MN.ctrlIn;
        lpi_client.upperLayerOut --> MN.in;
        MN.out --> lpi_client.upperLayerIn;
}
