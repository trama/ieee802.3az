

package eeepowerlink.deleted.simulations;

import eeepowerlink.nodes.ControlledNode;
import eeepowerlink.nodes.ManagingNode;
import eeepowerlink.src.EtherHubEEE;


network PowerLinkBigNetStar
{
    parameters:
        int dr @unit(Mbps) = default(100Mbps);

        @display("bgb=1068,805");
    submodules:
        Host1: ControlledNode {
            parameters:
                @display("p=227,279;is=l");
        }
        Host2: ControlledNode {
            parameters:
                @display("p=369,220;is=l");
        }
        PLC: ManagingNode {
            parameters:
                @display("p=530,73;is=l");
        }
        Host4: ControlledNode {
            parameters:
                @display("p=794,253;is=l");
        }
        Host3: ControlledNode {
            parameters:
                @display("p=898,348;is=l");
        }
        Host5: ControlledNode {
            parameters:
                @display("p=681,220;is=l");
        }
        Host6: ControlledNode {
            parameters:
                @display("p=820,636;is=l");
        }
        Host7: ControlledNode {
            parameters:
                @display("p=898,484;is=l");
        }
        Host8: ControlledNode {
            parameters:
                @display("p=183,565;is=l");
        }
        Host9: ControlledNode {
            parameters:
                @display("p=161,425;is=l");
        }
        Host10: ControlledNode {
            parameters:
                @display("p=242,657;is=l");
        }
        Host13: ControlledNode {
            parameters:
                @display("p=530,723;is=l");
        }
        Host11: ControlledNode {
            parameters:
                @display("p=369,695;is=l");
        }
        Host12: ControlledNode {
            parameters:
                @display("p=681,708;is=l");
        }
        hub5: EtherHubEEE {
            parameters:
                @display("p=530,454;is=l");
            gates:
                ethg[14];
        }

    connections:
        PLC.ethg <--> {  datarate = dr; } <--> hub5.ethg[0];

        hub5.ethg[1] <--> {  datarate = dr; } <--> Host1.ethg;
        hub5.ethg[2] <--> {  datarate = dr; } <--> Host2.ethg;
        hub5.ethg[3] <--> {  datarate = dr; } <--> Host3.ethg;
        hub5.ethg[4] <--> {  datarate = dr; } <--> Host4.ethg;
        hub5.ethg[5] <--> {  datarate = dr; } <--> Host5.ethg;
        hub5.ethg[6] <--> {  datarate = dr; } <--> Host6.ethg;
        hub5.ethg[7] <--> {  datarate = dr; } <--> Host7.ethg;
        hub5.ethg[8] <--> {  datarate = dr; } <--> Host8.ethg;
        hub5.ethg[9] <--> {  datarate = dr; } <--> Host9.ethg;
        hub5.ethg[10] <--> {  datarate = dr; } <--> Host10.ethg;
        hub5.ethg[11] <--> {  datarate = dr; } <--> Host11.ethg;
        hub5.ethg[12] <--> {  datarate = dr; } <--> Host12.ethg;
        hub5.ethg[13] <--> {  datarate = dr; } <--> Host13.ethg;
}
