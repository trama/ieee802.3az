package eeepowerlink.networks;

import eeepowerlink.nodes.IControlledNode;
import eeepowerlink.nodes.IControlledNodeDC;
import eeepowerlink.nodes.IManagingNode;
import eeepowerlink.nodes.ISwitchEEE;

network EPL_tii2014
{
    parameters:
        int numberOfCN = default(5);
        int n1 = default(20);
        int n2 = default(20);
        int numberOfSwitch = default(1);
        int dr @unit(bps) = default(100Mbps);
        bool daisychain = default(true);
        string CNDC_type = default("ControlledNodeEPL_DC");
        string CN_type = default("ControlledNodeEPL");
        string MN_type = default("ManagingNodeEPL");
        string SW_type @display("bgb=1150,1000;bgs=1,m") = default("switchEEE");
    submodules:
        PLC: <MN_type> like IManagingNode {
            parameters:
                @display("p=691,62;is=l");
        }
        switch0: <SW_type> like ISwitchEEE {
            parameters:
                @display("p=691,145;is=l");
            //gates:
            //    ethg[4];
        }
        switch1: <SW_type> like ISwitchEEE {
            @display("p=289,306");
            gates:
                ethg[n1+1];
        }
        switch2: <SW_type> like ISwitchEEE {
            @display("p=691,306");
            gates:
                ethg[n2+1];
        }
        Subnet1_Host[n1]: <CN_type> like IControlledNode {
            @display("p=68,477,m,4,70,100");
        }
        Subnet2_Host[n2]: <CN_type> like IControlledNode {
            @display("p=691,477,m,4,70,100");
        }
        DC_CN1: <CNDC_type> like IControlledNodeDC if daisychain {
            @display("p=930,157");
        }
        DC_CN2: <CNDC_type> like IControlledNodeDC if daisychain {
            parameters:
                terminator = true;
                @display("p=1078,157");
        }
    connections allowunconnected:
        switch0.ethg++ <--> {  datarate = dr; } <--> PLC.ethg;
        switch0.ethg++ <--> {  datarate = dr; } <--> switch1.ethg[0];
        switch0.ethg++ <--> {  datarate = dr; } <--> switch2.ethg[0];
        if daisychain{
            switch0.ethg++ <--> {  datarate = dr; } <--> DC_CN1.ethg++;
            DC_CN1.ethg++ <--> {  datarate = dr; } <--> DC_CN2.ethg++;
        }
        
        for i=0..n1-1 {
            switch1.ethg++ <--> {  datarate = dr; } <--> Subnet1_Host[i].ethg;
        }
        for i=0..n2-1 {
            switch2.ethg++ <--> {  datarate = dr; } <--> Subnet2_Host[i].ethg;
        }

}
