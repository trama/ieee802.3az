
package eeepowerlink.nodes;

import eeepowerlink.src.CN_daisychain.internalHub_EEE;
import eeepowerlink.src.EPL_ControlledNode;
import eeepowerlink.src.LPI_client;
import inet.linklayer.ethernet.EtherEncap;
import inet.linklayer.ethernet.EtherMACFullDuplex;

module ControlledNodeEPL_DC like IControlledNodeDC
{
    parameters:
        bool terminator = default(false);
    gates:
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        CN: EPL_ControlledNode {
            parameters:
                registerSAP = false;
                @display("p=131,62");
        }
        llc: EtherEncap {
            parameters:
                @display("p=131,221");
            //gates:
            //    upperLayerIn[1];
            //    upperLayerOut[1];
        }
        mac: EtherMACFullDuplex {
            parameters:
                duplexMode = true;
                @display("p=102,305;q=txQueue");
        }
        lpi_client: LPI_client {
            @display("p=131,145");
        }
        internalHub_EEE: internalHub_EEE {
            parameters:
                numOutputPorts = (terminator ? 1 : 2);
                @display("p=131,377");
        }
    connections:

        CN.out --> lpi_client.upperLayerIn;
        lpi_client.upperLayerOut --> CN.in;
        lpi_client.lowerLayerOut --> llc.upperLayerIn;
        llc.upperLayerOut --> lpi_client.lowerLayerIn;
        llc.lowerLayerOut --> mac.upperLayerIn;
        mac.upperLayerOut --> llc.lowerLayerIn;
        CN.ctrlOut --> lpi_client.upperControlIn;
        lpi_client.upperControlOut --> CN.ctrlIn;
        mac.phys <--> {  datarate = 1Gbps; } <--> internalHub_EEE.tomac;

        internalHub_EEE.ethg[0] <--> ethg++;
        internalHub_EEE.ethg[1] <--> ethg++ if !terminator;

}
