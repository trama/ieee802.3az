

package eeepowerlink.deleted.simulations;

import eeepowerlink.nodes.ControlledNode;
import eeepowerlink.nodes.ManagingNode;
import eeepowerlink.src.EtherHubEEE;


network PowerLinkBigNet
{
    parameters:
        int dr @unit(Mbps) = default(100Mbps);
        @display("bgb=1068,805");
    submodules:
        Host1: ControlledNode {
            parameters:
                @display("p=131,189;is=l");
        }
        Host2: ControlledNode {
            parameters:
                @display("p=146,81;is=l");
        }
        PLC: ManagingNode {
            parameters:
                @display("p=523,374;is=l");
        }
        hub: EtherHubEEE {
            parameters:
                @display("p=267,130;is=l");
            gates:
                ethg[4];
        }

        Host4: ControlledNode {
            parameters:
                @display("p=958,81;is=l");
        }
        Host3: ControlledNode {
            parameters:
                @display("p=905,245;is=l");
        }
        Host5: ControlledNode {
            parameters:
                @display("p=712,45;is=l");
        }
        hub1: EtherHubEEE {
            parameters:
                @display("p=823,140;is=l");
            gates:
                ethg[4];
        }

        Host6: ControlledNode {
            parameters:
                @display("p=942,541;is=l");
        }
        Host7: ControlledNode {
            parameters:
                @display("p=966,391;is=l");
        }
        hub2: EtherHubEEE {
            parameters:
                @display("p=823,401;is=l");
            gates:
                ethg[4];
        }

        Host8: ControlledNode {
            parameters:
                @display("p=199,519;is=l");
        }
        Host9: ControlledNode {
            parameters:
                @display("p=72,404;is=l");
        }
        Host10: ControlledNode {
            parameters:
                @display("p=361,433;is=l");
        }
        hub3: EtherHubEEE {
            parameters:
                @display("p=216,343;is=l");
            gates:
                ethg[4];
        }

        Host13: ControlledNode {
            parameters:
                @display("p=551,735;is=l");
        }
        Host11: ControlledNode {
            parameters:
                @display("p=413,647;is=l");
        }
        Host12: ControlledNode {
            parameters:
                @display("p=716,697;is=l");
        }
        hub4: EtherHubEEE {
            parameters:
                @display("p=582,615;is=l");
            gates:
                ethg[4];
        }

        hub5: EtherHubEEE {
            parameters:
                @display("p=523,245;is=l");
            gates:
                ethg[4];
        }

    connections:
        hub.ethg[0] <--> {  datarate = dr; } <--> hub5.ethg[1];
        hub.ethg[3] <--> {  datarate = dr; } <--> hub3.ethg[0];
        hub.ethg[1] <--> {  datarate = dr; } <--> Host2.ethg;
        hub.ethg[2] <--> {  datarate = dr; } <--> Host1.ethg;


        hub1.ethg[1] <--> {  datarate = dr; } <--> Host3.ethg;
        hub1.ethg[2] <--> {  datarate = dr; } <--> Host4.ethg;
        hub1.ethg[3] <--> {  datarate = dr; } <--> Host5.ethg;

        hub2.ethg[2] <--> {  datarate = dr; } <--> Host6.ethg;
        hub2.ethg[3] <--> {  datarate = dr; } <--> Host7.ethg;
        hub2.ethg[1] <--> {  datarate = dr; } <--> hub4.ethg[0];


        hub3.ethg[1] <--> {  datarate = dr; } <--> Host9.ethg;
        hub3.ethg[2] <--> {  datarate = dr; } <--> Host8.ethg;
        hub3.ethg[3] <--> {  datarate = dr; } <--> Host10.ethg;

        hub4.ethg[1] <--> {  datarate = dr; } <--> Host11.ethg;
        hub4.ethg[2] <--> {  datarate = dr; } <--> Host13.ethg;
        hub4.ethg[3] <--> {  datarate = dr; } <--> Host12.ethg;


        hub5.ethg[2] <--> {  datarate = dr; } <--> hub1.ethg[0];
        hub5.ethg[3] <--> {  datarate = dr; } <--> hub2.ethg[0];

        PLC.ethg <--> {  datarate = dr; } <--> hub5.ethg[0];

}
