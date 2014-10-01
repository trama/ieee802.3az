

package eeepowerlink.deleted.nodes;

import eeepowerlink.nodes.EtherPowHost;
import eeepowerlink.nodes.ManagingNode;
import inet.linklayer.ethernet.EtherHub;


network HubLAN
{
    @display("bgb=315,298");
    submodules:
        Host1: EtherPowHost {
            parameters:
                @display("p=159,211");
        }
        Host2: EtherPowHost {
            parameters:
                @display("p=33,250");
        }
        PLC: ManagingNode {
            parameters:
                @display("p=159,35");
        }
        Host3: EtherPowHost {
            parameters:
                @display("p=281,250");
        }
        hub: EtherHub {
            parameters:
                @display("p=159,130");
            gates:
                ethg[4];
        }

    connections:
        hub.ethg[3] <--> {  delay = 0.2us; datarate = 100Mbps; } <--> PLC.ethg;
        hub.ethg[0] <--> {  delay = 0.3us; datarate = 100Mbps; } <--> Host2.ethg;
        hub.ethg[1] <--> {  delay = 0.4us; datarate = 100Mbps; } <--> Host1.ethg;
        hub.ethg[2] <--> {  delay = 0.4us; datarate = 100Mbps; } <--> Host3.ethg;

}
