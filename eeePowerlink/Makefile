#
# OMNeT++/OMNEST Makefile for eeePowerlink
#
# This file was generated with the command:
#  opp_makemake -f --deep -O out -Xdeleted -I/Users/federico/simulazioni/inet/src/util/headerserializers/tcp -I/Users/federico/simulazioni/inet/src/linklayer/ethernet -I/Users/federico/simulazioni/inet/src/networklayer/ipv4 -I/Users/federico/simulazioni/inet/src/networklayer/common -I/Users/federico/simulazioni/inet/src/util/headerserializers/ipv4 -I/Users/federico/simulazioni/inet/src/base -I/Users/federico/simulazioni/inet/src/applications/ethernet -I/Users/federico/simulazioni/inet/src/util/headerserializers -I/Users/federico/simulazioni/inet/src/mobility -I/Users/federico/simulazioni/inet/src/networklayer/contract -I/Users/federico/simulazioni/inet/src/networklayer/autorouting/ipv4 -I/Users/federico/simulazioni/inet/src/util -I/Users/federico/simulazioni/inet/src/transport/contract -I/Users/federico/simulazioni/inet/src/linklayer/common -I/Users/federico/simulazioni/inet/src/status -I/Users/federico/simulazioni/inet/src/util/headerserializers/udp -I/Users/federico/simulazioni/inet/src/applications/pingapp -I/Users/federico/simulazioni/inet/src/linklayer/contract -I/Users/federico/simulazioni/inet/src/networklayer/arp -I/Users/federico/simulazioni/inet/src/transport/tcp_common -I/Users/federico/simulazioni/inet/src/transport/udp -I/Users/federico/simulazioni/inet/src/linklayer/ethernet/switch -L/Users/federico/simulazioni/inet/out/$$\(CONFIGNAME\)/src -linet -DINET_IMPORT -KINET_PROJ=/Users/federico/simulazioni/inet
#

# Name of target to be created (-o option)
TARGET = eeePowerlink$(EXE_SUFFIX)

# User interface (uncomment one) (-u option)
USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(TKENV_LIBS) $(CMDENV_LIBS)
#USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(TKENV_LIBS)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I$(INET_PROJ)/src/util/headerserializers/tcp \
    -I$(INET_PROJ)/src/linklayer/ethernet \
    -I$(INET_PROJ)/src/networklayer/ipv4 \
    -I$(INET_PROJ)/src/networklayer/common \
    -I$(INET_PROJ)/src/util/headerserializers/ipv4 \
    -I$(INET_PROJ)/src/base \
    -I$(INET_PROJ)/src/applications/ethernet \
    -I$(INET_PROJ)/src/util/headerserializers \
    -I$(INET_PROJ)/src/mobility \
    -I$(INET_PROJ)/src/networklayer/contract \
    -I$(INET_PROJ)/src/networklayer/autorouting/ipv4 \
    -I$(INET_PROJ)/src/util \
    -I$(INET_PROJ)/src/transport/contract \
    -I$(INET_PROJ)/src/linklayer/common \
    -I$(INET_PROJ)/src/status \
    -I$(INET_PROJ)/src/util/headerserializers/udp \
    -I$(INET_PROJ)/src/applications/pingapp \
    -I$(INET_PROJ)/src/linklayer/contract \
    -I$(INET_PROJ)/src/networklayer/arp \
    -I$(INET_PROJ)/src/transport/tcp_common \
    -I$(INET_PROJ)/src/transport/udp \
    -I$(INET_PROJ)/src/linklayer/ethernet/switch \
    -I. \
    -Inetworks \
    -Inodes \
    -Isimulations \
    -Isimulations/matlab \
    -Isimulations/matlab/etfa2013 \
    -Isimulations/matlab/tii2014 \
    -Isimulations/old \
    -Isimulations/results \
    -Isimulations/tii2014_addr_table \
    -Isrc \
    -Isrc/CN_daisychain \
    -Isrc/basetx100

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = -L$(INET_PROJ)/out/$(CONFIGNAME)/src  -linet
LIBS += -Wl,-rpath,`abspath $(INET_PROJ)/out/$(CONFIGNAME)/src`

# Output directory
PROJECT_OUTPUT_DIR = out
PROJECTRELATIVE_PATH =
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc and .msg files
OBJS = \
    $O/src/EPL_ControlledNode.o \
    $O/src/EPL_ManagingNode.o \
    $O/src/LPIclient.o \
    $O/src/powerListener.o \
    $O/src/switchEEE.o \
    $O/src/CN_daisychain/internalHub_EEE.o \
    $O/src/CN_daisychain/internalHub_EEE_intel.o \
    $O/src/basetx100/EEE_EtherMACFullDuplex.o \
    $O/src/basetx100/EEE_EtherTrafGen.o \
    $O/src/basetx100/EPL_ControlledNode_intel.o \
    $O/src/basetx100/EPL_ManagingNode_intel.o \
    $O/src/basetx100/LPIclient_100baseTX.o \
    $O/src/basetx100/relayUnitForEEE.o \
    $O/src/basetx100/switchEEE_100baseTX.o \
    $O/src/EPLframe_m.o \
    $O/src/basetx100/hello_m.o

# Message files
MSGFILES = \
    src/EPLframe.msg \
    src/basetx100/hello.msg

# Other makefile variables (-K)
INET_PROJ=/Users/federico/simulazioni/inet

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc or configuser.vc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
ifneq ("$(OMNETPP_ROOT)","")
CONFIGFILE = $(OMNETPP_ROOT)/Makefile.inc
else
CONFIGFILE = $(shell opp_configfilepath)
endif
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIB_SUBDIR = $(OMNETPP_LIB_DIR)/$(TOOLCHAIN_NAME)
OMNETPP_LIBS = -L"$(OMNETPP_LIB_SUBDIR)" -L"$(OMNETPP_LIB_DIR)" -loppmain$D $(USERIF_LIBS) $(KERNEL_LIBS) $(SYS_LIBS)

COPTS = $(CFLAGS) -DINET_IMPORT $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE and have object
# files depend on it (except when "make depend" was called)
COPTS_FILE = $O/.last-copts
ifneq ($(MAKECMDGOALS),depend)
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
$(shell $(MKPATH) "$O" && echo "$(COPTS)" >$(COPTS_FILE))
endif
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
# >>>
# <<<
#------------------------------------------------------------------------------

# Main target
all: $O/$(TARGET)
	$(Q)$(LN) $O/$(TARGET) .

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile
	@$(MKPATH) $O
	@echo Creating executable: $@
	$(Q)$(CXX) $(LDFLAGS) -o $O/$(TARGET)  $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS)

.PHONY: all clean cleanall depend msgheaders

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(CXXFLAGS) $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cc $(MSGCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

clean:
	$(qecho) Cleaning...
	$(Q)-rm -rf $O
	$(Q)-rm -f eeePowerlink eeePowerlink.exe libeeePowerlink.so libeeePowerlink.a libeeePowerlink.dll libeeePowerlink.dylib
	$(Q)-rm -f ./*_m.cc ./*_m.h
	$(Q)-rm -f networks/*_m.cc networks/*_m.h
	$(Q)-rm -f nodes/*_m.cc nodes/*_m.h
	$(Q)-rm -f simulations/*_m.cc simulations/*_m.h
	$(Q)-rm -f simulations/matlab/*_m.cc simulations/matlab/*_m.h
	$(Q)-rm -f simulations/matlab/etfa2013/*_m.cc simulations/matlab/etfa2013/*_m.h
	$(Q)-rm -f simulations/matlab/tii2014/*_m.cc simulations/matlab/tii2014/*_m.h
	$(Q)-rm -f simulations/old/*_m.cc simulations/old/*_m.h
	$(Q)-rm -f simulations/results/*_m.cc simulations/results/*_m.h
	$(Q)-rm -f simulations/tii2014_addr_table/*_m.cc simulations/tii2014_addr_table/*_m.h
	$(Q)-rm -f src/*_m.cc src/*_m.h
	$(Q)-rm -f src/CN_daisychain/*_m.cc src/CN_daisychain/*_m.h
	$(Q)-rm -f src/basetx100/*_m.cc src/basetx100/*_m.h

cleanall: clean
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(qecho) Creating dependencies...
	$(Q)$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cc networks/*.cc nodes/*.cc simulations/*.cc simulations/matlab/*.cc simulations/matlab/etfa2013/*.cc simulations/matlab/tii2014/*.cc simulations/old/*.cc simulations/results/*.cc simulations/tii2014_addr_table/*.cc src/*.cc src/CN_daisychain/*.cc src/basetx100/*.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/src/EPL_ControlledNode.o: src/EPL_ControlledNode.cc \
	src/EPLDefs.h \
	src/EPL_ControlledNode.h \
	src/EPLframe_m.h \
	src/powerListener.h \
	$(INET_PROJ)/src/applications/ethernet/EtherApp_m.h \
	$(INET_PROJ)/src/base/ILifecycle.h \
	$(INET_PROJ)/src/base/INETDefs.h \
	$(INET_PROJ)/src/base/INotifiable.h \
	$(INET_PROJ)/src/base/IPassiveQueue.h \
	$(INET_PROJ)/src/linklayer/common/MACBase.h \
	$(INET_PROJ)/src/linklayer/contract/Ieee802Ctrl_m.h \
	$(INET_PROJ)/src/linklayer/contract/MACAddress.h \
	$(INET_PROJ)/src/linklayer/ethernet/EtherMACBase.h \
	$(INET_PROJ)/src/status/NodeStatus.h
$O/src/EPL_ManagingNode.o: src/EPL_ManagingNode.cc \
	src/EPLDefs.h \
	src/EPL_ManagingNode.h \
	src/EPLframe_m.h \
	src/powerListener.h \
	$(INET_PROJ)/src/applications/ethernet/EtherApp_m.h \
	$(INET_PROJ)/src/base/INETDefs.h \
	$(INET_PROJ)/src/linklayer/contract/Ieee802Ctrl_m.h \
	$(INET_PROJ)/src/linklayer/contract/MACAddress.h
$O/src/EPLframe_m.o: src/EPLframe_m.cc \
	src/EPLframe_m.h
$O/src/LPIclient.o: src/LPIclient.cc \
	src/EPLDefs.h \
	src/EPLframe_m.h \
	src/LPIclient.h \
	src/powerListener.h \
	$(INET_PROJ)/src/base/INETDefs.h
$O/src/powerListener.o: src/powerListener.cc \
	src/powerListener.h \
	$(INET_PROJ)/src/base/INETDefs.h
$O/src/switchEEE.o: src/switchEEE.cc \
	src/EPLDefs.h \
	src/EPLframe_m.h \
	src/powerListener.h \
	src/switchEEE.h \
	$(INET_PROJ)/src/base/INETDefs.h \
	$(INET_PROJ)/src/linklayer/contract/Ieee802Ctrl_m.h \
	$(INET_PROJ)/src/linklayer/contract/MACAddress.h \
	$(INET_PROJ)/src/linklayer/ethernet/EtherFrame_m.h \
	$(INET_PROJ)/src/linklayer/ethernet/Ethernet.h
$O/src/CN_daisychain/internalHub_EEE.o: src/CN_daisychain/internalHub_EEE.cc \
	src/CN_daisychain/internalHub_EEE.h \
	src/EPLDefs.h \
	src/EPLframe_m.h \
	src/powerListener.h \
	$(INET_PROJ)/src/base/ILifecycle.h \
	$(INET_PROJ)/src/base/INETDefs.h \
	$(INET_PROJ)/src/base/INotifiable.h \
	$(INET_PROJ)/src/base/IPassiveQueue.h \
	$(INET_PROJ)/src/linklayer/common/MACBase.h \
	$(INET_PROJ)/src/linklayer/contract/Ieee802Ctrl_m.h \
	$(INET_PROJ)/src/linklayer/contract/MACAddress.h \
	$(INET_PROJ)/src/linklayer/ethernet/EtherFrame_m.h \
	$(INET_PROJ)/src/linklayer/ethernet/EtherMACBase.h \
	$(INET_PROJ)/src/linklayer/ethernet/EtherMACFullDuplex.h \
	$(INET_PROJ)/src/linklayer/ethernet/Ethernet.h \
	$(INET_PROJ)/src/status/NodeStatus.h
$O/src/CN_daisychain/internalHub_EEE_intel.o: src/CN_daisychain/internalHub_EEE_intel.cc \
	src/CN_daisychain/internalHub_EEE_intel.h \
	src/EPLDefs.h \
	src/EPLframe_m.h \
	src/powerListener.h \
	$(INET_PROJ)/src/base/ILifecycle.h \
	$(INET_PROJ)/src/base/INETDefs.h \
	$(INET_PROJ)/src/linklayer/contract/Ieee802Ctrl_m.h \
	$(INET_PROJ)/src/linklayer/contract/MACAddress.h \
	$(INET_PROJ)/src/linklayer/ethernet/EtherFrame_m.h \
	$(INET_PROJ)/src/linklayer/ethernet/Ethernet.h \
	$(INET_PROJ)/src/linklayer/ethernet/switch/MACRelayUnitBase.h \
	$(INET_PROJ)/src/linklayer/ethernet/switch/MACRelayUnitNP.h
$O/src/basetx100/EEE_EtherMACFullDuplex.o: src/basetx100/EEE_EtherMACFullDuplex.cc \
	src/EPLDefs.h \
	src/basetx100/EEE_EtherMACFullDuplex.h \
	src/basetx100/hello_m.h \
	$(INET_PROJ)/src/base/ILifecycle.h \
	$(INET_PROJ)/src/base/INETDefs.h \
	$(INET_PROJ)/src/base/INotifiable.h \
	$(INET_PROJ)/src/base/IPassiveQueue.h \
	$(INET_PROJ)/src/base/ModuleAccess.h \
	$(INET_PROJ)/src/base/NotificationBoard.h \
	$(INET_PROJ)/src/base/NotifierConsts.h \
	$(INET_PROJ)/src/linklayer/common/MACBase.h \
	$(INET_PROJ)/src/linklayer/contract/Ieee802Ctrl_m.h \
	$(INET_PROJ)/src/linklayer/contract/MACAddress.h \
	$(INET_PROJ)/src/linklayer/ethernet/EtherFrame_m.h \
	$(INET_PROJ)/src/linklayer/ethernet/EtherMACBase.h \
	$(INET_PROJ)/src/linklayer/ethernet/EtherMACFullDuplex.h \
	$(INET_PROJ)/src/linklayer/ethernet/Ethernet.h \
	$(INET_PROJ)/src/networklayer/common/InterfaceEntry.h \
	$(INET_PROJ)/src/networklayer/common/InterfaceToken.h \
	$(INET_PROJ)/src/status/NodeStatus.h
$O/src/basetx100/EEE_EtherTrafGen.o: src/basetx100/EEE_EtherTrafGen.cc \
	src/basetx100/EEE_EtherTrafGen.h \
	$(INET_PROJ)/src/base/ILifecycle.h \
	$(INET_PROJ)/src/base/INETDefs.h \
	$(INET_PROJ)/src/base/LifecycleOperation.h \
	$(INET_PROJ)/src/base/ModuleAccess.h \
	$(INET_PROJ)/src/base/NodeOperations.h \
	$(INET_PROJ)/src/linklayer/contract/Ieee802Ctrl_m.h \
	$(INET_PROJ)/src/linklayer/contract/MACAddress.h \
	$(INET_PROJ)/src/status/NodeStatus.h
$O/src/basetx100/EPL_ControlledNode_intel.o: src/basetx100/EPL_ControlledNode_intel.cc \
	src/EPLDefs.h \
	src/EPLframe_m.h \
	src/basetx100/EPL_ControlledNode_intel.h \
	src/powerListener.h \
	$(INET_PROJ)/src/applications/ethernet/EtherApp_m.h \
	$(INET_PROJ)/src/base/ILifecycle.h \
	$(INET_PROJ)/src/base/INETDefs.h \
	$(INET_PROJ)/src/base/INotifiable.h \
	$(INET_PROJ)/src/base/IPassiveQueue.h \
	$(INET_PROJ)/src/linklayer/common/MACBase.h \
	$(INET_PROJ)/src/linklayer/contract/Ieee802Ctrl_m.h \
	$(INET_PROJ)/src/linklayer/contract/MACAddress.h \
	$(INET_PROJ)/src/linklayer/ethernet/EtherMACBase.h \
	$(INET_PROJ)/src/status/NodeStatus.h
$O/src/basetx100/EPL_ManagingNode_intel.o: src/basetx100/EPL_ManagingNode_intel.cc \
	src/EPLDefs.h \
	src/EPLframe_m.h \
	src/basetx100/EPL_ManagingNode_intel.h \
	src/powerListener.h \
	$(INET_PROJ)/src/applications/ethernet/EtherApp_m.h \
	$(INET_PROJ)/src/base/INETDefs.h \
	$(INET_PROJ)/src/linklayer/contract/Ieee802Ctrl_m.h \
	$(INET_PROJ)/src/linklayer/contract/MACAddress.h
$O/src/basetx100/LPIclient_100baseTX.o: src/basetx100/LPIclient_100baseTX.cc \
	src/EPLDefs.h \
	src/basetx100/LPIclient_100baseTX.h \
	src/basetx100/hello_m.h \
	src/powerListener.h \
	$(INET_PROJ)/src/base/INETDefs.h
$O/src/basetx100/hello_m.o: src/basetx100/hello_m.cc \
	src/basetx100/hello_m.h
$O/src/basetx100/relayUnitForEEE.o: src/basetx100/relayUnitForEEE.cc \
	src/EPLDefs.h \
	src/EPLframe_m.h \
	src/basetx100/relayUnitForEEE.h \
	src/powerListener.h \
	$(INET_PROJ)/src/base/ILifecycle.h \
	$(INET_PROJ)/src/base/INETDefs.h \
	$(INET_PROJ)/src/linklayer/contract/Ieee802Ctrl_m.h \
	$(INET_PROJ)/src/linklayer/contract/MACAddress.h \
	$(INET_PROJ)/src/linklayer/ethernet/EtherFrame_m.h \
	$(INET_PROJ)/src/linklayer/ethernet/Ethernet.h \
	$(INET_PROJ)/src/linklayer/ethernet/switch/MACRelayUnitBase.h \
	$(INET_PROJ)/src/linklayer/ethernet/switch/MACRelayUnitNP.h
$O/src/basetx100/switchEEE_100baseTX.o: src/basetx100/switchEEE_100baseTX.cc \
	src/EPLDefs.h \
	src/EPLframe_m.h \
	src/basetx100/switchEEE_100baseTX.h \
	src/powerListener.h \
	$(INET_PROJ)/src/base/INETDefs.h \
	$(INET_PROJ)/src/linklayer/contract/Ieee802Ctrl_m.h \
	$(INET_PROJ)/src/linklayer/contract/MACAddress.h \
	$(INET_PROJ)/src/linklayer/ethernet/EtherFrame_m.h \
	$(INET_PROJ)/src/linklayer/ethernet/Ethernet.h

