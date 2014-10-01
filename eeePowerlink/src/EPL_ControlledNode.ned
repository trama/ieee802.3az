
package eeepowerlink.src;

//
// Server side of the EtherCli model -- generates ~EtherAppResp packets
// with the number of bytes requested by the client in ~EtherAppReq.
// It should be connected directly to ~EtherLLC or a Ieee802NicXXX module.
//
simple EPL_ControlledNode
{
    parameters:
        double sleepTime @unit(s) = default(200us);
        double quietTime @unit(s) = default(20ms);
        double wakeTime @unit(s);
        double eplCycleTime @unit(s) = default(400us);  // Powerlink cycle period
        //double Tplt @unit(s) = default(5us);
        double TPreq_Pres_CN @unit(s) = default(20us);  // delay between end of PReq reception and start of PRes transmission                 	
        double propagationTime @unit(s) = default(20us);  // signal propagation time
        bool lpiOnSoA = default(true);
        bool partialLPI = default(false);
        int subNetIndex = default(-1);
        double numHost = default(0);  // number of the Host
        volatile int reqLength @unit(B) = default(1B);  // length of request packets
        
        bool registerSAP = default(false);   // whether to sent IEEE802CTRL_REGISTER_DSAP on startup
        bool autoSetMACaddress = default(false);
        
        double partialWakeTime @unit(s) = default(10us);
        
        @display("i=block/app");
        @signal[sentPk](type=cPacket);
        @signal[rcvdPk](type=cPacket);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input in @labels(Ieee802Ctrl/up);
        output out @labels(Ieee802Ctrl/down);
        input ctrlIn @labels(LPI);
        output ctrlOut @labels(LPI);
}
