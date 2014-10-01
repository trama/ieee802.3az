

package eeepowerlink.deleted.simulations;

network MultiplexedTimeslot
{
    parameters:
        int dr @unit(Mbps) = default(100Mbps);

        @display("bgb=731,792");
    submodules:
        MN: ManagingNode {
            parameters:
                @display("p=343,31");
        }
        hub2: EtherHubEEE {
            parameters:
                @display("p=343,374");
            gates:
                ethg[3];
        }

        hub1A1: EtherHubEEE {
            parameters:
                @display("p=468,171");
            gates:
                ethg[3];
        }

        hub3: EtherHubEEE {
            parameters:
                @display("p=343,584");
            gates:
                ethg[3];
        }

        hub1: EtherHubEEE {
            parameters:
                @display("p=343,171");
            gates:
                ethg[5];
        }

        hub1A2: EtherHubEEE {
            parameters:
                @display("p=579,171");
            gates:
                ethg[3];
        }

        hub1A3: EtherHubEEE {
            parameters:
                @display("p=689,171");
            gates:
                ethg[2];
        }

        Host3: ControlledNode_multiplexed {
            parameters:
                @display("p=467,82");
        }
        Host2: ControlledNode_multiplexed {
            parameters:
                @display("p=578,82");
        }
        Host1: ControlledNode_multiplexed {
            parameters:
                @display("p=688,82");
        }
        Host4: ControlledNode_multiplexed {
            parameters:
                @display("p=579,244");
        }
        Host5: ControlledNode_multiplexed {
            parameters:
                @display("p=468,244");
        }
        hub1B1: EtherHubEEE {
            parameters:
                @display("p=468,327");
            gates:
                ethg[3];
        }

        hub1B2: EtherHubEEE {
            parameters:
                @display("p=579,327");
            gates:
                ethg[2];
        }

        hub1C1: EtherHubEEE {
            parameters:
                @display("p=468,496");
            gates:
                ethg[3];
        }

        Host6: ControlledNode_multiplexed {
            parameters:
                @display("p=579,413");
        }
        hub1C2: EtherHubEEE {
            parameters:
                @display("p=579,496");
            gates:
                ethg[2];
        }

        Host7: ControlledNode_multiplexed {
            parameters:
                @display("p=468,413");
        }
        hub2A2: EtherHubEEE {
            parameters:
                @display("p=145,374");
            gates:
                ethg[2];
        }

        hub2A1: EtherHubEEE {
            parameters:
                @display("p=236,374");
            gates:
                ethg[3];
        }

        Host8: ControlledNode_multiplexed {
            parameters:
                @display("p=145,291");
        }
        Host9: ControlledNode_multiplexed {
            parameters:
                @display("p=236,291");
        }
        hub3B1: EtherHubEEE {
            parameters:
                @display("p=236,747");
            gates:
                ethg[3];
        }

        Host13: ControlledNode_multiplexed {
            parameters:
                @display("p=134,657");
        }
        hub3A3: EtherHubEEE {
            parameters:
                @display("p=44,584");
            gates:
                ethg[2];
        }

        Host10: ControlledNode_multiplexed {
            parameters:
                @display("p=44,495");
        }
        Host11: ControlledNode_multiplexed {
            parameters:
                @display("p=145,495");
        }
        hub3B2: EtherHubEEE {
            parameters:
                @display("p=134,747");
            gates:
                ethg[2];
        }

        hub3A1: EtherHubEEE {
            parameters:
                @display("p=236,584");
            gates:
                ethg[3];
        }

        Host14: ControlledNode_multiplexed {
            parameters:
                @display("p=236,657");
        }
        hub3A2: EtherHubEEE {
            parameters:
                @display("p=145,584");
            gates:
                ethg[3];
        }

        Host12: ControlledNode_multiplexed {
            parameters:
                @display("p=236,489");
        }
    connections:

        MN.ethg <--> {  datarate = dr; } <--> hub1.ethg[0];

        hub1.ethg[1] <--> {  datarate = dr; } <--> hub1A1.ethg[0];

        hub1A1.ethg[1] <--> {  datarate = dr; } <--> Host3.ethg;
        hub1A1.ethg[2] <--> {  datarate = dr; } <--> hub1A2.ethg[0];
        hub1A2.ethg[1] <--> {  datarate = dr; } <--> Host2.ethg;
        hub1A2.ethg[2] <--> {  datarate = dr; } <--> hub1A3.ethg[0];
        hub1A3.ethg[1] <--> {  datarate = dr; } <--> Host1.ethg;

        hub1.ethg[2] <--> {  datarate = dr; } <--> hub1B1.ethg[0];

        hub1B1.ethg[1] <--> {  datarate = dr; } <--> Host5.ethg;
        hub1B1.ethg[2] <--> {  datarate = dr; } <--> hub1B2.ethg[0];
        hub1B2.ethg[1] <--> {  datarate = dr; } <--> Host4.ethg;

        hub1.ethg[3] <--> {  datarate = dr; } <--> hub1C1.ethg[0];

        hub1C1.ethg[1] <--> {  datarate = dr; } <--> Host7.ethg;
        hub1C1.ethg[2] <--> {  datarate = dr; } <--> hub1C2.ethg[0];
        hub1C2.ethg[1] <--> {  datarate = dr; } <--> Host6.ethg;

        hub1.ethg[4] <--> {  datarate = dr; } <--> hub2.ethg[0];

        hub2.ethg[1] <--> {  datarate = dr; } <--> hub2A1.ethg[0];

        hub2A1.ethg[1] <--> {  datarate = dr; } <--> Host9.ethg;
        hub2A1.ethg[2] <--> {  datarate = dr; } <--> hub2A2.ethg[0];
        hub2A2.ethg[1] <--> {  datarate = dr; } <--> Host8.ethg;

        hub2.ethg[2] <--> {  datarate = dr; } <--> hub3.ethg[0];

        hub3.ethg[1] <--> {  datarate = dr; } <--> hub3A1.ethg[0];

        hub3A1.ethg[1] <--> {  datarate = dr; } <--> Host12.ethg;
        hub3A1.ethg[2] <--> {  datarate = dr; } <--> hub3A2.ethg[0];
        hub3A2.ethg[1] <--> {  datarate = dr; } <--> Host11.ethg;
        hub3A2.ethg[2] <--> {  datarate = dr; } <--> hub3A3.ethg[0];
        hub3A3.ethg[1] <--> {  datarate = dr; } <--> Host10.ethg;


        hub3.ethg[2] <--> {  datarate = dr; } <--> hub3B1.ethg[0];

        hub3B1.ethg[1] <--> {  datarate = dr; } <--> Host14.ethg;
        hub3B1.ethg[2] <--> {  datarate = dr; } <--> hub3B2.ethg[0];
        hub3B2.ethg[1] <--> {  datarate = dr; } <--> Host13.ethg;

}
