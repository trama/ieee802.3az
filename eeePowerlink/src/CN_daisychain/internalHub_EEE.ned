package eeepowerlink.src.CN_daisychain;

simple internalHub_EEE
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("i=device/hub");
        @signal[pk](type=cMessage);
        @statistic[pk](title="packets"; source=pk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        int numHUB = default(0);  // number of the Host
        int numOutputPorts = default(2);
        double powerConsumption @unit(mW) = default(0mW);
        double powerConsumptionEEE @unit(mW) = default(0mW);
        double eeePowerIncrement @unit(mW) = default(0mW);
        double pEeePowerIncrement @unit(mW) = default(0mW);
        double noEeePowerIncrement @unit(mW) = default(0mW);
        double eeeTransitionPower @unit(mW) = default(0mW);
        double pEeeTransitionPower @unit(mW) = default(0mW);
        double eplCycleTime @unit(s) = default(400us);  // Powerlink cycle period

        bool partialLPI = default(false);
        bool hasSubnetworks = default( (numOutputPorts == 2 ? true : false ) ); // typically a daisy chain CN can be considered as part of a subnetwork
        string MNaddress = default("aabbccddee02");
        double sleepTime @unit(s) = default(100us);
        double quietTime @unit(s) = default(20ms);
        double wakeTime @unit(s) = default(30us);
        double refreshTime @unit(s) = default(20ms);
        //string addressTableFile = default("");

    gates:
        inout tomac @labels(EtherFrame);
        inout ethg[numOutputPorts] @labels(EtherFrame-conn);  // to stations; each one represents a port
}
