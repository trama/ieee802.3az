
package eeepowerlink.src.basetx100;

import eeepowerlink.nodes.ISwitchEEE;
//
// A generic wiring hub model.
//
// ethg[i] gates represent ports. Messages arriving on a port are broadcast
// to every other port. For the model to work correctly, all connecting 
// links (both incoming and outgoing ones) must have the same datarate.
//
// Messages are not interpreted by the hub model in any way, thus the hub
// model is not specific to Ethernet. Messages may represent anything, from
// the beginning of a frame transmission to end (or abortion) of transmission.
//
// It is allowed to dynamically unconnect/reconnect ports of the hub, and also
// to change the size of ethg[] to add/remove ports. However, the model only
// checks the equivalence of input/output datarates on startup; you are
// responsible that links added at runtime have have the correct datarate.
//


simple switchEEE_100baseTX like ISwitchEEE
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("i=device/hub");
        @signal[pk](type=cMessage);
        @statistic[pk](title="packets"; source=pk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        int numHUB = default(0);  // number of the Host
        double powerConsumption @unit(mW) = default(0mW);
        double powerConsumptionEEE @unit(mW) = default(0mW);
        double eeePowerIncrement @unit(mW) = default(0mW);
        double pEeePowerIncrement @unit(mW) = default(0mW);
        double noEeePowerIncrement @unit(mW) = default(0mW);
        double eeeTransitionPower @unit(mW) = default(0mW);
        double pEeeTransitionPower @unit(mW) = default(0mW);
        double eplCycleTime @unit(s) = default(400us);  // Powerlink cycle period

        bool partialLPI = default(false);
        bool hasSubnetworks = default(false);
        double sleepTime @unit(s) = default(100us);
        double quietTime @unit(s) = default(20ms);
        double wakeTime @unit(s) = default(30us);
        double refreshTime @unit(s) = default(20ms);
        string addressTableFile = default("");

    gates:
        inout ethg[] @labels(EtherFrame-conn);  // to stations; each one represents a port
        //output etho[] @labels(EtherFrame-conn);  // to stations; each one represents a port
}
