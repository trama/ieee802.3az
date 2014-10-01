

package eeepowerlink.networks;

import eeepowerlink.nodes.ControlledNodeEPL;
import eeepowerlink.nodes.ManagingNodeEPL;
import eeepowerlink.src.switchEEE;


network EPL_etfa
{
    parameters:
        int numberOfCN = default(5);
        int n = default(20);
        int numberOfSwitch = default(1);
        int dr @unit(bps) = default(100Mbps);
        @display("bgb=2000,1000;bgl=2");
    submodules:
        Host0: ControlledNodeEPL {
            parameters:
                @display("p=115,216;is=l");
        }
        PLC: ManagingNodeEPL {
            parameters:
                @display("p=911,66;is=l");
        }
        Host1: ControlledNodeEPL {
            parameters:
                @display("p=1796,216;is=l");
        }
        switch0: switchEEE {
            parameters:
                @display("p=911,149;is=l");
            gates:
                ethg[4];
        }
        
        switch1: switchEEE {
            @display("p=261,216");
            gates:
                ethg[3];
        }
        switch2: switchEEE {
            @display("p=1619,216");
            gates:
                ethg[3];
        }
        switch3: switchEEE {
            @display("p=263,306");
            gates:
                ethg[n+1];
        }
        switch4: switchEEE {
            @display("p=911,310");
            gates:
                ethg[n+1];
        }
        switch5: switchEEE {
            @display("p=1621,306");
            gates:
                ethg[n+1];
        }
        Subnet1_Host[n]: ControlledNodeEPL {
            @display("p=46,481,m,7,70,100");
        }
        Subnet2_Host[n]: ControlledNodeEPL {
            @display("p=757,481,m,7,70,100");
        }
        Subnet3_Host[n]: ControlledNodeEPL {
            @display("p=1510,481,m,7,70,100");
        }
    connections allowunconnected:
        switch0.ethg[0] <--> {  datarate = dr; } <--> PLC.ethg;
        switch0.ethg[1] <--> {  datarate = dr; } <--> switch1.ethg[0];
        switch0.ethg[2] <--> {  datarate = dr; } <--> switch2.ethg[0];
        switch0.ethg[3] <--> {  datarate = dr; } <--> switch4.ethg[0];
        
        switch1.ethg[1] <--> {  datarate = dr; } <--> Host0.ethg;
        switch2.ethg[1] <--> {  datarate = dr; } <--> Host1.ethg;
        
        switch5.ethg[0] <--> {  datarate = dr; } <--> switch2.ethg[2];
        switch3.ethg[0] <--> {  datarate = dr; } <--> switch1.ethg[2];
        for i=0..n-1 {
            switch3.ethg++ <--> {  datarate = dr; } <--> Subnet1_Host[i].ethg;
        }
        for i=0..n-1 {
            switch4.ethg++ <--> {  datarate = dr; } <--> Subnet2_Host[i].ethg;
        }
        for i=0..n-1 {
            switch5.ethg++ <--> {  datarate = dr; } <--> Subnet3_Host[i].ethg;
        }

}
