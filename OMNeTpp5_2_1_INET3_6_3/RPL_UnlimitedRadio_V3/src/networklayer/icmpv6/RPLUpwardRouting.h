/***************************************************************************
 * file:        RPLRouting.h
 *
 * author:      Hamidreza Kermajani
 *
 * copyright:   (C) 2013 UPC, Castelldefels, Spain.
 *
 * description: Implementation of the IPv6 Routing Protocol for Low power
 *              and Lossy Networks (RPL).
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 *
 ***************************************************************************
 * last modification: 09/29/2013
 **************************************************************************/

/*
 * Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2), Carles Gomez(3);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
 *                    (3) UPC, Castelldefels, Spain.
 *
 *                    Adapted for using on INET 3.6.3, and also included some changes such as using ICMPv6 messages
 *                    for transmitting RPL control messages, interface table , and lifesycle modules
 *
 * To read more information about the Kermajani's article, you can use [1].
 *
 *                    [1] Kermajani, Hamidreza, and Carles Gomez. "On the network convergence process
 *                    in RPL over IEEE 802.15. 4 multihop networks: Improvement and trade-offs."
 *                    Sensors 14.7 (2014): 11993-12022.�
 */


#ifndef _RPL_SRC_NETWORKLAYER_ICMPV6_RPLUPWARDROUTING_H
#define _RPL_SRC_NETWORKLAYER_ICMPV6_RPLUPWARDROUTING_H

#include <map>

#include "src/statisticcollector/StatisticCollector.h"
#include "src/networklayer/icmpv6/ICMPv6MessageRPL_m.h"
//#include "src/networklayer/icmpv6/ICMPv6RPL.h" loop
#include "inet/common/INETDefs.h"
#include "src/networklayer/contract/RPLDefs.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv6/IPv6AddressType.h"
//#include "inet/networklayer/contract/IRoutingTable.h" //doesn't support addDefaultRote()
#include "inet/networklayer/ipv6/IPv6RoutingTable.h"

#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/common/lifecycle/NodeStatus.h"

#include "src/simulationManager/ManagerRPL.h"
#include "src/networklayer/icmpv6/ParentTableRPL.h"
#include "src/networklayer/icmpv6/IPv6NeighbourDiscoveryRPL.h"
#include "src/networklayer/icmpv6/SourceRoutingTable.h"

namespace rpl {
using namespace inet;

/**
 * @brief IPv6 Routing Protocol for LLNs (RPL)provides a mechanism whereby
 * multipoint-to-point traffic from devices inside the LLN towards a central
 * control point is supported.
 *
**/

class ICMPv6RPL;

class RPLUpwardRouting : public cSimpleModule, public ILifecycle//, public INetfilter::IHook//, public cListener

{
protected:

    // environment
    IPv6NeighbourDiscoveryRPL *neighbourDiscoveryRPL;
    IPv6RoutingTable *routingTable;
    SourceRoutingTable *sourceRoutingTable;
    IInterfaceTable *interfaceTable;
    ManagerRPL *pManagerRPL;
    StatisticCollector *statisticCollector;
    INetfilter *networkProtocol;
    ICMPv6RPL *icmpv6RPL;
    ParentTableRPL *parentTableRPL;
    InterfaceEntry *ie;
    int interfaceID;

    /** @brief Gate ids */
    //@{
    int icmpv6InGateId;
    int icmpv6OutGateId;
    //@}

    // lifecycle
    bool isOperational;

    /**
     * @brief Length of the NetwPkt header
     * Read from omnetpp.ini
     **/
    int DIOheaderLength;
    int headerLength;

    /**
     * @brief RPL setting parameters
     * Read from omnetpp.ini
     **/

    RPLMOP mop; //Mode Of Operation
    bool DISEnable;
    bool refreshDAORoutes;
    simtime_t DelayDAO;
    simtime_t defaultLifeTime;

    double DIOIntMin;
    int DIORedun;
    int DIOIntDoubl;
    simtime_t DIOIMaxLength;

    cModule *host;

    MACAddress macaddress;

    IPv6Address sinkLinkLocalAddress; //Link local addres
    IPv6Address sinkGlobalAddress; //Global address
    IPv6Address myLLNetwAddr; //Link Local address
    IPv6Address myGlobalNetwAddr; //Global Address
    IPv6Address DODAGID;
    IPv6Address PrParent;


    bool isSink;

    cMessage* DIOTimer;

    bool isJoinedFirstVersion;
    bool isNodeJoined;

    unsigned char dtsnInstance;

    int Rank;
    simtime_t NodeStartTime;
    int versionNember;
    int Grounded;
    simtime_t TimetoSendDIO;

    int DIO_c;
    simtime_t DIO_CurIntsizeNow,DIO_CurIntsizeNext;
    simtime_t DIO_StofCurIntNow,DIO_StofCurIntNext;
    simtime_t DIO_EndofCurIntNow,DIO_EndofCurIntNext;

    /** @brief Statistics */
    //@{
    int numSentDIO;
    int numReceivedDIO;
    int numSuppressedDIO;
    //@}


public:
    /** @brief Copy constructor is not allowed.
     */
   // RPLRouting(const RPLRouting&);
    /** @brief Assignment operator is not allowed.
     */
   // RPLRouting& operator=(const RPLRouting&);


    RPLUpwardRouting()
        : neighbourDiscoveryRPL(nullptr)
        , routingTable(nullptr)
        , sourceRoutingTable(nullptr)
        , interfaceTable (nullptr)
        , pManagerRPL (nullptr)
        , statisticCollector (nullptr)
        , networkProtocol (nullptr)
        , icmpv6RPL(nullptr)
        , parentTableRPL(nullptr)
        , ie(nullptr)
        , interfaceID(-1)
        , icmpv6InGateId(-1)
        , icmpv6OutGateId(-1)
        , isOperational (false)
        , DIOheaderLength(-1)
        , headerLength(-1)
        , mop(Storing_Mode_of_Operation_with_no_multicast_support)
        , DISEnable(false)
        , refreshDAORoutes(false)
        , DelayDAO(0)
        , defaultLifeTime(0)
        , DIOIntMin(0)
        , DIORedun(0)
        , DIOIntDoubl(0)
        , DIOIMaxLength(0)
        , host(nullptr)
        , macaddress(MACAddress::UNSPECIFIED_ADDRESS)
        , sinkLinkLocalAddress(IPv6Address::UNSPECIFIED_ADDRESS)
        , sinkGlobalAddress(IPv6Address::UNSPECIFIED_ADDRESS)
        , myLLNetwAddr(IPv6Address::UNSPECIFIED_ADDRESS)
        , myGlobalNetwAddr(IPv6Address::UNSPECIFIED_ADDRESS)
        , DODAGID(IPv6Address::UNSPECIFIED_ADDRESS)
        , PrParent(IPv6Address::UNSPECIFIED_ADDRESS)
        , isSink(false)
        , DIOTimer(nullptr)
        , isJoinedFirstVersion(false)
        , isNodeJoined(false)
        , dtsnInstance(0)
        , Rank(0)
        , NodeStartTime(0)
        , versionNember(0)
        , Grounded(0)
        , TimetoSendDIO(0)
        , DIO_c(0)
        , DIO_CurIntsizeNow(0)
        , DIO_CurIntsizeNext(0)
        , DIO_StofCurIntNow(0)
        , DIO_StofCurIntNext(0)
        , DIO_EndofCurIntNow(0)
        , DIO_EndofCurIntNext(0)
        , numSentDIO(0)
        , numReceivedDIO(0)
        , numSuppressedDIO(0)
    {};

    virtual ~RPLUpwardRouting();

protected:
    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    virtual void handleMessage(cMessage* msg) override;

    virtual void finish() override;

    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage* msg);


    /** @brief Handle messages from the ICMPv6 module */
    virtual void handleIncommingMessage(cMessage* msg);

    virtual void handleIncommingDIOMessage(cMessage* msg);

    /** @brief Handle self messages */
    virtual void handleSelfMsg(cMessage* msg);


    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage* msg);

    /** @brief Decapsulate a message */
    cMessage* decapsMsg(ICMPv6DIOMsg *msg);

public:
    /** @brief scheduling next DIO message transmission. */
    virtual void scheduleNextDIOTransmission();

public:
    void TrickleReset();
    void DeleteDIOTimer();

protected:
    void SetDIOParameters();

    virtual void handleDIOTimer(cMessage* msg);

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

    virtual void updateRoutingTable(IPv6Address &PrParent);


public:

    //The methods which is called in the statisticsCollector module
    virtual void setParametersBeforeGlobalRepair(simtime_t dodagSartTime);

    //The methods which is called in the icmpv6 module
    virtual bool isNodeJoinedToDAG() const;
    virtual int getVersion() const;
    virtual IPv6Address getDODAGID() const;
    virtual IPv6Address getMyLLNetwAddr() const;
    virtual IPv6Address getMyGlobalNetwAddr() const;
    virtual int getInterfaceID() const;
    virtual void getDIOStatistics(int &numSentDIO, int &numReceivedDIO, int &numSuppressedDIO) const;
};

} // namespace rpl

#endif  // ifndef _RPL_SRC_NETWORKLAYER_ICMPV6_RPLUPWARDROUTING_H

