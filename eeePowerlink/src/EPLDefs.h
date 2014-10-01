
#include "INETDefs.h"

#ifndef EVT
#define EVT (ev.isDisabled()||!debug) ? ev : ev << simTime().str() << "::" << logName() << "::"// << getClassName() << ": "
#endif

enum EEESelfMessageKind {
    START_LPI = 0,
    QUIET = 1,
    WAKE_UP = 2,
    WAKED_UP = 3,
    FULL_ACTIVE = 4,
    SEND_PRES = 5,
    START_POW = 6,
    SOA_GOLPI = 7,
    SEND_SOC = 8,
    POLLING = 9,
    SEND_SOA = 10,
    PREQ_TIMEOUT = 11,
    START_LPITX = 12,
    WAKE_AFTER = 13,
    START_EPL = 14,
    END_IFG,
    REFRESH,
    START_LPI_TX,
    START_LPI_RX
};

enum EEE_states{
    ACTIVE_TX = 111,
    ACTIVE_RX,
    ACTIVE_FULL,
    QUIET_TX,
    QUIET_RX,
    QUIET_FULL,
    TRANSITION_TX,
    TRANSITION_RX
};
