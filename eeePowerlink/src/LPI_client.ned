
package eeepowerlink.src;

//
// LPI_client implementation
//
simple LPI_client
{
    parameters:
        @display("i=block/layer");
        double sleepTime @unit(s);
        double quietTime @unit(s);
        double wakeTime @unit(s);
        double refreshTime @unit(s);
        double partialWakeTime @unit(s);
        double partialSleepTime @unit(s);
        
        double powerConsumption @unit(mW) = default(0mW);
        double powerConsumptionEEE @unit(mW) = default(0mW);
        double powerConsumptionPEEE @unit(mW) = default(0mW);
        double eeePowerIncrement @unit(mW) = default(0mW);
        double pEeePowerIncrement @unit(mW) = default(0mW);
        double noEeePowerIncrement @unit(mW) = default(0mW);
        double eeeTransitionPower @unit(mW) = default(0mW);
        double pEeeTransitionPower @unit(mW) = default(0mW);
//        double eeePowerDecrement @unit(mW) = default(0mW);
//        double pEeePowerDecrement @unit(mW) = default(0mW);
//        double noEeePowerDecrement @unit(mW) = default(0mW);
        @signal[lpi_time](type=simtime_t);
        @statistic[linkLPI_time](source=lpi_time; record=min,max,timeavg,vector);
        @signal[partial_lpi_time](type=simtime_t);
        @statistic[linkPLPI_time](source=partial_lpi_time; record=min,max,timeavg,vector);
        bool debug =default(false);
        bool partialLPI = default(false);
        bool iAmTheMn = default(false);
        
        gates:
        input upperLayerIn;    // from ~EtherLLC
        output upperLayerOut;  // to ~EtherLLC
        input upperControlIn @labels(LPI);    // from ~EtherLLC
        output upperControlOut @labels(LPI);  // to ~EtherLLC
        input lowerLayerIn @labels(Ieee802Ctrl/down);    // from ~EtherLLC
        output lowerLayerOut @labels(Ieee802Ctrl/up);  // to ~EtherLLC
}
