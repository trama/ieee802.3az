
package eeepowerlink.nodes;

import eeepowerlink.nodes.IControlledNode;

import eeepowerlink.src.EPL_ControlledNode;
import eeepowerlink.src.LPI_client;
import inet.linklayer.IEtherMAC;
import inet.linklayer.ethernet.EtherEncap;

module ControlledNodeEPL like IControlledNode
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
        CN: EPL_ControlledNode {
            parameters:
                registerSAP = false;
                @display("p=155,119");
        }
        llc: EtherEncap {
            parameters:
                @display("p=55,305");
            //gates:
            //    upperLayerIn[1];
            //    upperLayerOut[1];
        }
        mac: <macType> like IEtherMAC {
            parameters:
                queueModule = (queueType == "" ? "" : "queue");
                @display("p=155,305;q=txQueue");
        }
        lpi_client: LPI_client {
            @display("p=155,222");
        }
    connections:

        mac.phys <--> ethg;
        CN.out --> lpi_client.upperLayerIn;
        lpi_client.upperLayerOut --> CN.in;
        lpi_client.lowerLayerOut --> llc.upperLayerIn;
        llc.upperLayerOut --> lpi_client.lowerLayerIn;
        llc.lowerLayerOut --> mac.upperLayerIn;
        mac.upperLayerOut --> llc.lowerLayerIn;
        CN.ctrlOut --> lpi_client.upperControlIn;
        lpi_client.upperControlOut --> CN.ctrlIn;
}
