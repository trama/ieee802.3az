

package eeepowerlink.deleted.simulations;

import eeepowerlink.nodes.ControlledNode;
import eeepowerlink.nodes.ManagingNode;
import inet.linklayer.ethernet.EtherHub;


network PowerLinkNigNet
{
    parameters:
        int dr @unit(Mbps) = default(100Mbps);

        @display("bgb=403,306");
    submodules:
        Host1: ControlledNode {
            parameters:
                @display("p=202,228");
        }
        Host2: ControlledNode {
            parameters:
                @display("p=76,267");
        }
        PLC: ManagingNode {
            parameters:
                @display("p=202,52");
        }
        Host3: ControlledNode {
            parameters:
                @display("p=324,267");
        }
        hub: EtherHub {
            parameters:
                @display("p=202,147");
            gates:
                ethg[4];
        }

    connections:
        hub.ethg[3] <--> {  datarate = dr; } <--> PLC.ethg;
        hub.ethg[0] <--> {  datarate = dr; } <--> Host2.ethg;
        hub.ethg[1] <--> {  datarate = dr; } <--> Host1.ethg;
        hub.ethg[2] <--> {  datarate = dr; } <--> Host3.ethg;

}
