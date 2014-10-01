

package eeepowerlink.deleted.simulations;

import eeepowerlink.nodes.ControlledNode_Powerlink;
import eeepowerlink.nodes.ManagingNode_noEEE;
import inet.linklayer.ethernet.EtherHub;


network PowerLinkSmallNet
{
    parameters:
        int dr @unit(Mbps) = default(100Mbps);
        @display("bgb=1068,805");
    submodules:
        PLC: ManagingNode_noEEE {
            parameters:
                @display("p=523,130;is=l");
        }
        Host3: ControlledNode_Powerlink {
            parameters:
                @display("p=628,521;is=l");
        }
        Host2: ControlledNode_Powerlink {
            parameters:
                @display("p=464,511;is=l");
        }
        Host1: ControlledNode_Powerlink {
            parameters:
                @display("p=326,483;is=l");
        }
        hub1: EtherHub {
            parameters:
                @display("p=485,398;is=l");
            gates:
                ethg[4];
        }

    connections:
        hub1.ethg[1] <--> {  datarate = dr; } <--> Host1.ethg;
        hub1.ethg[2] <--> {  datarate = dr; } <--> Host2.ethg;
        hub1.ethg[3] <--> {  datarate = dr; } <--> Host3.ethg;

        PLC.ethg <--> {  datarate = dr; } <--> hub1.ethg[0];
}
