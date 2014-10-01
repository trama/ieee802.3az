

package eeepowerlink.networks;

import eeepowerlink.nodes.ControlledNodeEPL;
import eeepowerlink.nodes.ManagingNodeEPL;
import eeepowerlink.src.switchEEE;


network EPL_tase
{
    parameters:
        int numberOfCN = default(5);
        int n = default(20);
        int numberOfSwitch = default(1);
        int dr @unit(bps) = default(100Mbps);
        @display("bgb=490,402;bgl=2");
    submodules:
        PLC: ManagingNodeEPL {
            parameters:
                @display("p=46,155;is=l");
        }
        Host1: ControlledNodeEPL {
            parameters:
                @display("p=225,58;is=l");
        }
        switch0: switchEEE {
            parameters:
                @display("p=225,155;is=l");
        }

        Host2: ControlledNodeEPL {
            parameters:
                @display("p=364,96;is=l");
        }
        Host3: ControlledNodeEPL {
            parameters:
                @display("p=364,231;is=l");
        }
        Host4: ControlledNodeEPL {
            parameters:
                @display("p=225,272;is=l");
        }
    connections allowunconnected:
        switch0.ethg++ <--> {  datarate = dr; } <--> PLC.ethg;
        Host1.ethg <--> {  datarate = dr; } <--> switch0.ethg++;
        switch0.ethg++ <--> {  datarate = dr; } <--> Host2.ethg;
        switch0.ethg++ <--> {  datarate = dr; } <--> Host3.ethg;
        switch0.ethg++ <--> {  datarate = dr; } <--> Host4.ethg;
}
