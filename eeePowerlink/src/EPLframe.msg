//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

//
// TODO generated message class
//
packet EPLframe {
    int type @enum(EPLframeMessageKind); // frame kind
    int host; // PReq Host Receiver
    int mp; // cycleCount
    bool partialLPI;
}

enum EPLframeMessageKind 
{
    PREQ = 0;
    PRES = 1;
    SOA = 2;
    SOC = 3;
    ASYNC = 4;
    STOP_IDLE = 5;
    LPI_SIGNAL = 6;
    EXIT_PLPI = 7;
    RETURN_PLPI = 8;
};