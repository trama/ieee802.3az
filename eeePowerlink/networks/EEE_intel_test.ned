//
// Copyright (C) 2014, F. Tramarin
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package eeepowerlink.networks;

import eeepowerlink.nodes.ControlledNodeEPL_intel;
import eeepowerlink.nodes.ManagingNodeEPL_intel;
import eeepowerlink.src.basetx100.EEE_EtherSwitch;
import ned.DatarateChannel;


network EEE_intel_test
{
    parameters:
        
        int dr @unit(bps) = default(100Mbps);
        int numberOfCN = default(5);
        int numberOfSwitch = default(1);
    types:
        channel ethernetline extends DatarateChannel
        {
            parameters:
                delay = 0.1us; //cavo di 20 m, considerando una vel di propag di 2e8 m/s
                datarate = 100Mbps;
        }
    submodules:
        PLC: ManagingNodeEPL_intel {
            parameters:
                @display("p=161,55");
        }
        Host1: ControlledNodeEPL_intel {
            parameters:
                @display("p=253,125");
        }
        switch: EEE_EtherSwitch {
            parameters:
                @display("p=162,124");
            gates:
                ethg[3];
        }
        Host2: ControlledNodeEPL_intel {
            parameters:
                @display("p=50,131");
        }
    connections:
        switch.ethg[0] <--> ethernetline <--> PLC.ethg;
        switch.ethg[1] <--> ethernetline <--> Host1.ethg;
        switch.ethg[2] <--> ethernetline <--> Host2.ethg;
}
