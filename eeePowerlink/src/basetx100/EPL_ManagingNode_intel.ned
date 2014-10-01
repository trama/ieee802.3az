
package eeepowerlink.src.basetx100;

//
// A simple traffic generator for the Ethernet model and the 802.11 model,
// and generally for any L2 model that accepts Ieee80211Ctrl control info
// on the packets. It generates ~EtherAppReq packets. It should be connected
// directly to ~EtherLLC or a Ieee802NicXXX module.
//
simple EPL_ManagingNode_intel
{
    parameters:

        double eplCycleTime @unit(s) = default(1ms);  // Powerlink cycle period
        double eplPreqPresTime @unit(s) = default(20us);  // delay between end of PReq reception and start of PRes transmission                 	
        double propagationTime @unit(s) = default(20us);  // signal propagation time
        double wakeTime @unit(s);
        double partialWakeTime @unit(s);
        double TPreq_Pres_MN @unit(s)= default(2us);
        
        volatile int reqLength @unit(B) = default(1B);  // length of request packets
        bool registerSAP = default(false);   // whether to sent IEEE802CTRL_REGISTER_DSAP on startup
        double startTime @unit(s) = default(0s);
        double stopTime @unit(s) = default(1s);
        int numberOfSubnetworks = default(0);
        //int nodePerSubnet = default(20);
        //int midLevelNode = default(0);
        bool lpiOnSoA = default(true);
        bool partialLPI = default(false);
        bool multiplexed = default(false);
        double muxEPLtime @unit(s) = default(-1s);
        string muxCN = default("");
        @display("i=block/app");
        @signal[sentPk](type=cPacket);
        @signal[rcvdPk](type=cPacket);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input in @labels(Ieee802Ctrl/up);
        output out @labels(Ieee802Ctrl/down);
}

