

package eeepowerlink.networks;

import eeepowerlink.nodes.ControlledNodeEPL;
import eeepowerlink.nodes.ManagingNodeEPL;
import eeepowerlink.src.switchEEE;


network EPL_test
{
    parameters:
        int numberOfCN = default(5);
        int numberOfSwitch = default(1);
        int dr @unit(bps) = default(100Mbps);
        @display("bgb=443,372");
    submodules:
        Host0: ControlledNodeEPL {
            parameters:
                @display("p=56,227;is=l");
        }
        PLC: ManagingNodeEPL {
            parameters:
                @display("p=343,238;is=l");
        }
        switch: switchEEE {
            parameters:
                @display("p=181,161;is=l");
            gates:
                ethg[3];
        }

        Host1: ControlledNodeEPL {
            parameters:
                @display("p=200,280;is=l");
        }
    connections:
        switch.ethg[0] <--> {  datarate = dr; } <--> PLC.ethg;
        switch.ethg[1] <--> {  datarate = dr; } <--> Host0.ethg;
        switch.ethg[2] <--> {  datarate = dr; } <--> Host1.ethg;



}
