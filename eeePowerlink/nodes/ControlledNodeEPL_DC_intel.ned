
package eeepowerlink.nodes;

import eeepowerlink.src.CN_daisychain.internalHub_EEE;
import eeepowerlink.src.CN_daisychain.internalHub_EEE_intel;
import eeepowerlink.src.basetx100.EEE_EtherMACFullDuplex;
import eeepowerlink.src.basetx100.EPL_ControlledNode_intel;
import eeepowerlink.src.basetx100.LPI_client_100baseTX;
import inet.linklayer.ethernet.EtherEncap;


module ControlledNodeEPL_DC_intel like IControlledNodeDC
{
    parameters:
        bool terminator = default(false);
        @display("bgb=272,452");
    gates:
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        CN: EPL_ControlledNode_intel {
            parameters:
                @display("p=196,73");
        }
        llc: EtherEncap {
            parameters:
                @display("p=196,187");
            //gates:
            //    upperLayerIn[1];
            //    upperLayerOut[1];
        }
        internalHub_EEE_intel: internalHub_EEE_intel {
            @display("p=135,348");
        }
    connections:

        llc.upperLayerOut --> CN.in;
        CN.out --> llc.upperLayerIn;
        internalHub_EEE_intel.ethg[0] <--> ethg++;
        internalHub_EEE_intel.ethg[1] <--> ethg++ if terminator;
        internalHub_EEE_intel.tollc --> llc.lowerLayerIn;
        llc.lowerLayerOut --> internalHub_EEE_intel.fromllc;
}
