/***************************************************************************
 * file:        RPLRouting.cc
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
 *                    adapted for using on INET 3.6.3, and also included some changes such as using ICMPv6 messages
 *                    for transmitting RPL control messages, interface table , and lifesycle modules
*/



//EXTRA BEGIN
/*
#include "RPL.h"
#include <limits>
#include <algorithm>
#include <cassert>
#include "NetwControlInfo.h"
#include "MacToNetwControlInfo.h"
#include "ArpInterface.h"
#include "FindModule.h"
#include "DIOMsg_m.h"
#include "SimpleBattery.h"
#include "BatteryStats.h"
#include "ConnectionManager.h"
#include "PhyLayer.h"
#include "MobilityBase.h"
#include "BaseDecider.h"
*/

#include "inet/networklayer/ipv6/IPv6Route.h"
#include "inet/networklayer/common/IPSocket.h"
#include "inet/networklayer/ipv6/IPv6Route.h"
#include "inet/networklayer/contract/ipv6/IPv6ControlInfo.h"
#include "inet/networklayer/ipv6/IPv6InterfaceData.h"

#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/ModuleAccess.h"
#include "src/routing/rpl/RPLRouting.h"
//#include "src/simulationManager/managerRPL.h"

namespace rpl {
using namespace inet;
//EXTRA END
class managerRPL;
char Path[100],MainPath[100],Mode[4] = "a+t",K_value[3];
bool* IsNodeJoined,GlobalRepairTimerReset=false,Flg=false,IsFirsttime=true;
bool NeighboursCalculationFlag=false,IsDODAGFormed=false;
simtime_t DODAGSartTime,AvgDODAGFomationTime;
int AllDIOsSent=0,AllDIOsReceived=0,AllDIOsSuppressed=0,AvgAllDIOsSent=0;
int AvgAllDIOsReceived=0,AvgAllDIOsSuppressed=0;
//EXTRA BEGIN
//int Version,NodesNumber,NumberofIterations,*NodeCounter,GRT_Counter=0;
int Version,NodesNumber,NumberofIterations,*NodeCounter=nullptr,GRT_Counter=0;  //initializing nullptr to deallocate memory
//EXTRA END
static int NofDODAGformationNormal=0;
double AvgAllCollisionNarmal=0;
FILE *JoiningTime,*Collosion,*DIOSent,*DISSent,*FormationTime,*PacketLost,*NodesRank,*ConsumedPower;
FILE *IterationsNumber;
FILE *preferedParent, *numberOfParents, *numTableEntris;

RPLRouting **NodesAddress;

struct SubDataStr
{
    float *JoiningTime,*ConsumedPower;
    int *NodesRank;
};
struct DataStructure
{
    SubDataStr *OtherFields;
    float *FormationTime;
    int *Collosion,*DIOSent,*DISSent,*PacketLost;
    int IterationsNumber;
    //EXTRA BEGIN, variables for saving the number of table entries in each iteration
    int *numPreferedParents = nullptr;
    int *numParents = nullptr;
    //EXTRA END
}FileRecord;
int FileRecordCounter=-1;

void FileRecordMemoryAllocation(void)
{
    FileRecord.Collosion        = new int [NumberofIterations+10];
    FileRecord.DIOSent          = new int [NumberofIterations+10];
    FileRecord.DISSent          = new int [NumberofIterations+10];
    FileRecord.PacketLost       = new int [NumberofIterations+10];
    FileRecord.FormationTime    = new float [NumberofIterations+10];
    //EXTRA BEGIN, variables for saving the number of table entries
    FileRecord.numPreferedParents = new int[NumberofIterations+10];
    FileRecord.numParents = new int[NumberofIterations+10];
    //EXTRA END

    FileRecord.OtherFields      = new SubDataStr [NumberofIterations+10];
    for(int i =0;i<NumberofIterations+1;i++)
    {
        FileRecord.OtherFields[i].NodesRank     = new int [NodesNumber+10];
        FileRecord.OtherFields[i].JoiningTime   = new float [NodesNumber+10];
        FileRecord.OtherFields[i].ConsumedPower = new float [NodesNumber+10];
    }
}
void FileRecordMemoryDeallocation(void)
{
    //EXTRA BEGIN
 /*   for(int i =0;i<NumberofIterations+1;i++)
    {
        delete [] FileRecord.OtherFields[i].NodesRank;
        delete [] FileRecord.OtherFields[i].JoiningTime;
        delete [] FileRecord.OtherFields[i].ConsumedPower;
    }
    delete [] FileRecord.Collosion;
    delete [] FileRecord.DIOSent;
    delete [] FileRecord.DISSent;
    delete [] FileRecord.PacketLost;
    delete [] FileRecord.FormationTime;
    delete [] FileRecord.OtherFields;
*/
    if (FileRecord.OtherFields){
        for(int i =0;i<NumberofIterations+1;i++)
        {
            if (!FileRecord.OtherFields[i].NodesRank) delete [] FileRecord.OtherFields[i].NodesRank;
            if (!FileRecord.OtherFields[i].JoiningTime) delete [] FileRecord.OtherFields[i].JoiningTime;
            if (!FileRecord.OtherFields[i].ConsumedPower) delete [] FileRecord.OtherFields[i].ConsumedPower;
        }
        delete [] FileRecord.OtherFields;
        FileRecord.OtherFields = nullptr;  //for mutual exclusion
    }

    if (!FileRecord.Collosion){
        delete [] FileRecord.Collosion;
        FileRecord.Collosion = nullptr;
    }
    if (!FileRecord.DIOSent){
        delete [] FileRecord.DIOSent;
        FileRecord.DIOSent = nullptr;
    }
    if (!FileRecord.DISSent){
        delete [] FileRecord.DISSent;
        FileRecord.DISSent = nullptr;
    }
    if (!FileRecord.PacketLost){
        delete [] FileRecord.PacketLost;
        FileRecord.PacketLost = nullptr;
    }
    if (!FileRecord.FormationTime){
        delete [] FileRecord.FormationTime;
        FileRecord.FormationTime = nullptr;
    }
    //EXTRA BEGIN
    if (!FileRecord.numPreferedParents){
        delete [] FileRecord.numPreferedParents;
        FileRecord.numPreferedParents = nullptr;  //for mutual exclusion
    }

    if (!FileRecord.numParents){
        delete [] FileRecord.numParents;
        FileRecord.numParents = nullptr;  //for mutual exclusion
    }
    //EXTRA END

    //EXTRA END

}
void Datasaving(int,bool);

NodeState *CreateNewNodeState(int Index, int VersionNo, simtime_t Time, int NodeRank)
{
    NodeState* Temp;
    Temp = new NodeState;

    Temp->Rank = new int[NodesNumber];
    Temp->JoiningDODAGTime = new simtime_t[NodesNumber];
    Temp->PowerConsumption = new double[NodesNumber];

    Temp->Version = VersionNo;
    Temp->DIO.Sent=0;
    Temp->DIO.Received = 0;
    Temp->DIO.Suppressed = 0;
    Temp->DIS.Sent=0;
    Temp->DIS.Received = 0;
    Temp->DIS.Suppressed = 0;
    Temp->Collision = 0;
    Temp->PacketLost = 0;
    Temp->Rank[Index] = NodeRank;
    Temp->JoiningDODAGTime[Index] = Time;
    Temp->DODAGsFormationTimeRecords = 0;
    Temp->PowerConsumption[Index] = 0;

    //EXTRA BEGIN
    Temp->numPreferedParents = 0;
    Temp->numParents = 0;
    //EXTRA END

    Temp->Link=NULL;
    return Temp;
}
//EXTRA BEGIN
/*
char * SetPath(char* MainPath,char* FileName,char* KValue)
{
    char TempPath[100];
    strcpy(TempPath,MainPath);
    strcat(TempPath,FileName);
    strcat(TempPath,KValue);
    strcat(TempPath,".txt");
    return TempPath;
}
*/

char * SetPath(char* MainPath, const char* FileName, char* KValue)
{
    char *TempPath = new char[100];
    strcpy(TempPath,MainPath);
    strcat(TempPath,FileName);
    strcat(TempPath,KValue);
    strcat(TempPath,".txt");
    return TempPath;
}
//EXTRA END

Define_Module(RPLRouting);

void RPLRouting::initialize(int stage)
{
    //BaseNetwLayer::initialize(stage);  //EXTRA
    if (stage == INITSTAGE_LOCAL)  //if(stage == 0)  //EXTRA
    {
//EXTRA BEGIN
        host = getContainingNode(this);
        routingTable = getModuleFromPar<IRoutingTable>(par("routingTableModule"), this);
        interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        networkProtocol = getModuleFromPar<INetfilter>(par("networkProtocolModule"), this);
        pManagerRPL = check_and_cast<managerRPL *>(getSimulation()->getSystemModule()->getSubmodule("managerRPL"));

        icmpv6InGateId = findGate("icmpv6In");
        icmpv6OutGateId = findGate("icmpv6Out");


        //macaddress = arp->getMacAddr(myNetwAddr);
        sinkAddress = IPv6Address(par("sinkAddress"));  //sinkAddress = LAddress::L3Type( getParentModule()->getParentModule()->par("SinkNodeAddr").longValue() );  //EXTRA
        EV << "sink address is " << sinkAddress << endl;
        //headerLength = par ("headerLength");
        //EXTRA END
        DIOheaderLength = par ("DIOheaderLength");
        DISheaderLength = par ("DISheaderLength");
        //EXTRA BEGIN
        //rssiThreshold = par("rssiThreshold");
        //stats = par("stats");
        //trace = par("trace");
        //debug = par("debug");
        //useSimTracer = par("useSimTracer");
        //EXTRA END

        DIOIntMin = par ("DIOIntMin");
        DIOIntDoubl = par ("DIOIntDoubl");
        DIORedun = par ("DIORedun");
        DISStartDelay = par("DISStartDelay");
        DISIntMin = par ("DISIntMin");
        DISIntDoubl = par ("DISIntDoubl");
        DISRedun = par ("DISRedun");
        DISEnable = par ("DISEnable");
        NodeStartTime=par( "RPLStartTime" );  //NodeStartTime=getParentModule()->par( "NodeStartTime" );  //EXTRA
        MaxNofParents = par ("MaxNofParents");
        GlobalRepairTimer = par ("GlobalRepairTimer");
        NodesNumber=getParentModule()->getParentModule()->par( "numHosts" );  //NodesNumber=getParentModule()->getParentModule()->par( "numNodes" );  //EXTRA
        NumberofIterations = par ("NumberofIterations");
        itoa(DIORedun, K_value, 10);
        strcpy(MainPath,par("FilePath").stringValue());  //strcpy(MainPath,par("FilePath"));  //EXTRA
        NofParents = new int[NumberofIterations+2];
        Parents = new ParentStructure* [NumberofIterations+2];
        for( int i = 0 ; i < NumberofIterations+2 ; i++ )
            Parents[i] = new ParentStructure [MaxNofParents];
        if(IsFirsttime)
        {
            IsFirsttime = false;
            NodesAddress = new RPLRouting* [NodesNumber] ;
            IsNodeJoined = new bool[NodesNumber+1];
            for(int i=0;i<NodesNumber+1;i++) IsNodeJoined[i] = false;
            NodeCounter= new int[NumberofIterations+2];
            for(int i=0;i<NumberofIterations+2;i++) NodeCounter[i] = 0;
            FileRecordMemoryAllocation();
        }
        //EXTRA BEGIN
        //NodesAddress[getParentModule()->getIndex()] = this;
        //NodesAddress[pManagerRPL->getIndexFromAddress(myNetwAddr)] = this; this edition of previos line must be in "INITSTAGE_NETWORK_LAYER_3" and after initializing "myNetwAddr" stage because the stage of managerRPL module is "INITSTAGE_NETWORK_LAYER_3"
    }
    else if (stage == INITSTAGE_NETWORK_LAYER_3){
        int nInterfaces = interfaceTable->getNumInterfaces();
        if(nInterfaces > 2) //If host has more than 2 interfaces...
            error("The host has more than 2 interfaces (one connected and loopback) and that's still not implemented!");
        for (int k=0; k<nInterfaces; k++)
        {
            InterfaceEntry *ie = interfaceTable->getInterface(k);
            //We add only the info about the entry which is not the loopback
            if (!ie->isLoopback())
            {
                interfaceID = ie->getInterfaceId();
                myNetwAddr = ie->ipv6Data()->getLinkLocalAddress();
                EV << "my link local address is: " << myNetwAddr << endl;

            }
        }
        NodesAddress[pManagerRPL->getIndexFromAddress(myNetwAddr)] = this;
        EV << "my address is " << myNetwAddr <<"; my index in the topology is " << pManagerRPL->getIndexFromAddress(myNetwAddr) << endl;


    }
     else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(host->getSubmodule("status"));
        isOperational = !nodeStatus || nodeStatus->getState() == NodeStatus::UP;

        if (isOperational){
            //RoutingTable = NULL;
            hasRoute = new bool[NumberofIterations+2];
            for(int i=0; i<NumberofIterations+2; i++)
                hasRoute[i] = false;
            //EXTRA END
            NofEntry=0;
            DIOStatusHeader = NULL;
            DISStatusHeader = NULL;
            DODAGJoinTimeHeader = NULL;
            GRepairTimer = NULL;
            DIOTimer = NULL;
            DISTimer = NULL;
            DODAGSartTime=simTime();
            AvgDODAGFomationTime=simTime();
            IsJoined=false;
            VersionNember=-1;
            PrParent = IPv6Address::UNSPECIFIED_ADDRESS; //PrParent=-1; //EXTRA
            DIOIMaxLength=DIOIntMin* int(pow (2.0,DIOIntDoubl));
            DISIMaxLength=DISIntMin* int(pow (2.0,DISIntDoubl));
            for(int i=0;i<NumberofIterations+2;i++)
            {
                NofParents[i]=0;
                for(int j=0;j<MaxNofParents;j++)
                {
                    Parents[i][j].ParentId= IPv6Address::UNSPECIFIED_ADDRESS;  //Parents[i][j].ParentId=-1;   //EXTRA
                    Parents[i][j].ParentRank=-1;
                }
            }

            // Scheduling the sink node to the first DIO transmission!!
            //cModule* host=findHost();  //EXTRA
            DODAGSartTime=simTime();
            DIOTimer = new cMessage("DIO-timer", SEND_DIO_TIMER);

           if (myNetwAddr==sinkAddress)
            {
                AvgDODAGFomationTime=simTime();
                IsJoined=true;
                //EXTRA BEGIN
                //IsNodeJoined[sinkAddress]=true;
                IsNodeJoined[pManagerRPL->getIndexFromAddress(sinkAddress)]=true;
                //EXTRA END
                NodeStartTime=simTime();
                VersionNember=1;
                Version=VersionNember;
                NodeCounter[VersionNember]++;
                EV << "NodeCounter[" << VersionNember << "] = " << NodeCounter[VersionNember] << endl;  //EXTRA
                DIOStatusNew = CreateNewVersionDIO();
                DIOStatusLast = DIOStatusNew;
                DIOStatusHeader = DIOStatusNew;
                Rank=1;
                DODAGID=myNetwAddr;
                Grounded=1;
                DODAGJoinTimeNew = CreateNewVersionJoiningTime();
                DODAGJoinTimeNew->TimetoJoinDODAG = simTime();
                DODAGJoinTimeLast = DODAGJoinTimeNew;
                DODAGJoinTimeHeader = DODAGJoinTimeNew;

                DIO_CurIntsizeNext=DIOIntMin;
                DIO_StofCurIntNext=DODAGSartTime;
                DIO_EndofCurIntNext=DIO_StofCurIntNext+DIO_CurIntsizeNext;

                scheduleNextDIOTransmission();
                char buf[100];
                sprintf(buf,"Root");
                host->getDisplayString().setTagArg("t", 0, buf);
                //EXTRA BEGIN
                //NodeStateNew = CreateNewNodeState(myNetwAddr,VersionNember,simTime(),Rank);
                //NodeStateNew->JoiningDODAGTime[myNetwAddr] = DODAGJoinTimeNew->TimetoJoinDODAG;
                NodeStateNew = CreateNewNodeState(pManagerRPL->getIndexFromAddress(myNetwAddr),VersionNember,simTime(),Rank);
                NodeStateNew->JoiningDODAGTime[pManagerRPL->getIndexFromAddress(myNetwAddr)] = DODAGJoinTimeNew->TimetoJoinDODAG;
                //EXTRA END
                NodeStateLast = NodeStateNew;
                NodeStateHeader = NodeStateNew;
                if(GlobalRepairTimer!=0)
                {
                    GRepairTimer = new cMessage("GRepair-timer", Global_REPAIR_TIMER);
                    scheduleAt(GlobalRepairTimer,GRepairTimer );
                }
                //EXTRA BEGIN, is not available yet
                /*SimpleBattery* batt;
                batt = FindModule<SimpleBattery*>::findSubModule(findHost());
                for (int i=0; i<NodesNumber;i++)
                    NodeStateLast->PowerConsumption[i]=batt->CalculateRemainenergy(i);
                */
                //EXTRA END
          }
            else
            {
                char buf[100];
                sprintf(buf,"Not joined!");
                host->getDisplayString().setTagArg("t", 0, buf);
                if (DISEnable)
                {
                    DISTimer = new cMessage("DIS-timer", SEND_DIS_FLOOD_TIMER);
                    DIS_CurIntsizeNext=DISIntMin;
                    DIS_StofCurIntNext =DISStartDelay +DODAGSartTime;
                    DIS_EndofCurIntNext=DIS_StofCurIntNext+DIS_CurIntsizeNext;
                    DISHandler();
                    scheduleNextDISTransmission();
                }
            }

        } //end operational
     } //end stage routing
}


void RPLRouting::ScheduleNextGlobalRepair()
{
    //EXTRA BEGIN
    /*
    SimpleBattery* batt;
    batt = FindModule<SimpleBattery*>::findSubModule(findHost());

    */
    //ResetnbFramesWithInterference();
    //ResetnbFramesWithoutInterference();
    //ResetNBFramesWithInterferenceDropped();
    //ResetNBFramesWithoutInterferenceDropped();
    //EXTRA END

    for(int i=0;i<NodesNumber;i++)
        IsNodeJoined[i] = false;
    //EXTRA BEGIN
    //IsNodeJoined[sinkAddress]=true;
    IsNodeJoined[pManagerRPL->getIndexFromAddress(sinkAddress)]=true;
    //EXTRA END
    VersionNember++;
    Version=VersionNember;
    NodeCounter[Version]++;
    EV << "NodeCounter[" << VersionNember << "] = " << NodeCounter[VersionNember] << endl;  //EXTRA


    DIOStatusNew = CreateNewVersionDIO();
    DIOStatusLast->link = DIOStatusNew;
    DIOStatusLast = DIOStatusNew;

    Rank=1;
    DODAGID=myNetwAddr;

    Grounded=1;
    DODAGJoinTimeNew = CreateNewVersionJoiningTime();
    DODAGJoinTimeNew->TimetoJoinDODAG=simTime();
    DODAGJoinTimeLast->link = DODAGJoinTimeNew;
    DODAGJoinTimeLast = DODAGJoinTimeNew;

    DODAGSartTime=DODAGJoinTimeLast->TimetoJoinDODAG;
    IsDODAGFormed= false;
    NodeStateNew = new NodeState;
    //EXTRA BEGIN
    //NodeStateNew = CreateNewNodeState(myNetwAddr,VersionNember,simTime(),Rank);
    //NodeStateNew->JoiningDODAGTime[myNetwAddr] = DODAGJoinTimeLast->TimetoJoinDODAG;
    NodeStateNew = CreateNewNodeState(pManagerRPL->getIndexFromAddress(myNetwAddr),VersionNember,simTime(),Rank);
    NodeStateNew->JoiningDODAGTime[pManagerRPL->getIndexFromAddress(myNetwAddr)] = DODAGJoinTimeLast->TimetoJoinDODAG;
    //EXTRA END

    if(NodeStateHeader==NULL)
    {
        NodeStateLast = NodeStateNew;
        NodeStateHeader = NodeStateNew;
    }
    else
    {
        NodeStateLast ->Link = NodeStateNew;
        NodeStateLast = NodeStateNew;
    }

//EXTRA BEGIN  available in the MiXim implemetation
    //for (int i=0; i<NodesNumber;i++)
        //NodeStateLast->PowerConsumption[i]=batt->CalculateRemainenergy(i);
//EXTRA END
    DIO_CurIntsizeNext=DIOIntMin;
    DIO_StofCurIntNext=DODAGSartTime;
    DIO_EndofCurIntNext=DIO_StofCurIntNext+DIO_CurIntsizeNext;
    for (int i=0; i<NodesNumber;i++)
        NodesAddress[i]->DeleteDIOTimer();
    if (DISEnable)
        for (int i=0; i<NodesNumber;i++)
        {
            NodesAddress[i]->SetDISParameters();
            NodesAddress[i]->DISHandler();
        }
    scheduleAt(simTime()+GlobalRepairTimer,GRepairTimer );
    GRT_Counter++;
}

void RPLRouting::DeleteScheduledNextGlobalRepair()
{
    Enter_Method("DeleteScheduledNextGlobalRepair()");

    cancelEvent(GRepairTimer);
    scheduleAt(simTime(),GRepairTimer );
}

void RPLRouting::scheduleNextDIOTransmission()
{

    DIO_CurIntsizeNow = DIO_CurIntsizeNext;
    DIO_StofCurIntNow = DIO_StofCurIntNext;
    DIO_EndofCurIntNow = DIO_EndofCurIntNext;
    TimetoSendDIO=DIO_StofCurIntNow+uniform(0,DIO_CurIntsizeNow/2)+(DIO_CurIntsizeNow/2);
    scheduleAt(TimetoSendDIO,DIOTimer );
    DIO_CurIntsizeNext*=2;
    if (DIO_CurIntsizeNext>DIOIMaxLength) DIO_CurIntsizeNext=DIOIMaxLength;
    DIO_StofCurIntNext = DIO_EndofCurIntNext;
    DIO_EndofCurIntNext=DIO_StofCurIntNext+DIO_CurIntsizeNext;
    DIO_c=0;
}
void RPLRouting::TrickleReset()
{
    Enter_Method("TrickleReset()");
    if (DIOTimer){     //test1111
        cancelAndDelete(DIOTimer);
    }
    DIOTimer = new cMessage("DIO-timer", SEND_DIO_TIMER);
    DIO_CurIntsizeNext=DIOIntMin;
    DIO_StofCurIntNext=simTime();
    DIO_EndofCurIntNext=DIO_StofCurIntNext+DIO_CurIntsizeNext;
}
void RPLRouting::DeleteDIOTimer()
{
    Enter_Method("DeleteDIOTimer()");
    if (DIOTimer){     //test1111
        cancelAndDelete(DIOTimer);
    }
    DIOTimer = new cMessage("DIO-timer", SEND_DIO_TIMER);
}

void RPLRouting::SetDISParameters()
{
    Enter_Method("SetDISParameters()");
    cancelAndDelete(DISTimer);
    DISTimer = new cMessage("DIS-timer", SEND_DIS_FLOOD_TIMER);
    DIS_CurIntsizeNext=DISIntMin;
    DIS_StofCurIntNext =DISStartDelay+DODAGSartTime;
    DIS_EndofCurIntNext=DIS_StofCurIntNext+DIS_CurIntsizeNext;
}

void RPLRouting::scheduleNextDISTransmission()
{
    Enter_Method("scheduleNextDISTransmission()");
    DIS_CurIntsizeNow = DIS_CurIntsizeNext;
    DIS_StofCurIntNow = DIS_StofCurIntNext;
    DIS_EndofCurIntNow = DIS_EndofCurIntNext;
    TimetoSendDIS=DIS_StofCurIntNow+uniform(0,DIS_CurIntsizeNow/2)+(DIS_CurIntsizeNow/2);
    scheduleAt(TimetoSendDIS,DISTimer );
    DIS_CurIntsizeNext*=2;
    if (DIS_CurIntsizeNext>DISIMaxLength) DIS_CurIntsizeNext=DISIMaxLength;
    DIS_StofCurIntNext = DIS_EndofCurIntNext;
    DIS_EndofCurIntNext=DIS_StofCurIntNext+DIS_CurIntsizeNext;
    DIS_c=0;
}

//EXTRA BEGIN
void RPLRouting::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage())
        handleSelfMsg(msg);
    else
        handleIncommingMessage(msg);

}
//EXTRA END
void RPLRouting::handleSelfMsg(cMessage* msg)
{
    if (msg->getKind() == SEND_DIO_TIMER)
    {
        if(((DIO_c<DIORedun)&&(Version==VersionNember))||(DIORedun==0))
        {
            // Broadcast DIO message
            //EXTRA BEGIN
            ICMPv6DIOMsg* pkt = new ICMPv6DIOMsg("DIO", DIO);
            IPv6ControlInfo *controlInfo = new IPv6ControlInfo;
            controlInfo->setSrcAddr(myNetwAddr);
            controlInfo->setDestAddr(IPv6Address::ALL_NODES_2); // IPv6Address::ALL_NODES_2 is ff02::2 (scope 2 (link-local)), FIXME: ff02::1a for rpl
            //pkt->setInitialSrcAddr(myNetwAddr);
            //pkt->setFinalDestAddr(LAddress::L3BROADCAST);
            //pkt->setSrcAddr(myNetwAddr);
            //pkt->setDestAddr(LAddress::L3BROADCAST);
            //EXTRA END
            pkt->setByteLength(DIOheaderLength);
            pkt->setVersionNumber(VersionNember);
            pkt->setRank(Rank);
            pkt->setDODAGID(DODAGID);
            pkt->setGrounded(Grounded);
            pkt->setIMin(DIOIntMin);
            pkt->setNofDoub(DIOIntDoubl);
            pkt->setK(DIORedun);
            //EXTRA BEGIN
            //setDownControlInfo(pkt, LAddress::L2BROADCAST);
            //sendDown(pkt);
            pkt->setType(ICMPv6_RPL_CONTROL_MESSAGE);
            controlInfo->setProtocol(IP_PROT_IPv6_ICMP);
            pkt->setControlInfo(controlInfo);
            send(pkt, icmpv6OutGateId);
            //EXTRA END
            if ((NodeCounter[Version]<NodesNumber)&&(!IsDODAGFormed))  NodeStateLast->DIO.Sent++;
            DIOStatusLast->nbDIOSent++;
            char buf[100];
            if(myNetwAddr==sinkAddress)
                 sprintf(buf,"DODAG ROOT\nVerNum = %d\nRank = %d\nnbDIOSent = %d\nnbDIOReceived = %d\nnbDIOSuppressed = %d",VersionNember,Rank,DIOStatusLast->nbDIOSent,DIOStatusLast->nbDIOReceived,DIOStatusLast->nbDIOSuppressed);
            else
                //EXTRA BEGIN
                //sprintf(buf,"Joined!\nVerNum = %d\nRank = %d\nPrf.Parent = %d\nnbDIOSent = %d\nnbDIOReceived = %d\nnbDIOSuppressed = %d",VersionNember,Rank,int(PrParent),DIOStatusLast->nbDIOSent,DIOStatusLast->nbDIOReceived,DIOStatusLast->nbDIOSuppressed);
                sprintf(buf,"Joined!\nVerNum = %d\nRank = %d\nPrf.Parent = %s\nnbDIOSent = %d\nnbDIOReceived = %d\nnbDIOSuppressed = %d",VersionNember,Rank,PrParent.getSuffix(96).str().c_str(),DIOStatusLast->nbDIOSent,DIOStatusLast->nbDIOReceived,DIOStatusLast->nbDIOSuppressed);
            //EXTRA END
            //cModule* host=findHost(); //EXTRA; host has been declared in the initial method
            host->getDisplayString().setTagArg("t", 0, buf);
            cancelAndDelete(DIOTimer);
            DIOTimer = new cMessage("DIO-timer", SEND_DIO_TIMER);
            scheduleNextDIOTransmission();
            return;
        }
        else
        {
            if((DIO_c>=DIORedun)&&(Version==VersionNember))
            {
                if ((NodeCounter[Version]<NodesNumber)&&(!IsDODAGFormed))  NodeStateLast->DIO.Suppressed++;

                DIOStatusLast->nbDIOSuppressed++;
                char buf1[100];
                sprintf(buf1, "DIO transmission suppressed!");
                //cModule* host=findHost();  //EXTRA; host was defined in initiat method
                host->bubble(buf1);
                //delete msg;  //EXTRA reason of error cancelAndDelete(DIOTimer);
                char buf2[100];
                //EXTRA BEGIN
                //sprintf(buf2,"Joined!\nVerNum = %d\nRank = %d\nPrf.Parent = %d\nnbDIOSent = %d\nnbDIOReceived = %d\nnbDIOSuppressed = %d",VersionNember,Rank,int(PrParent),DIOStatusLast->nbDIOSent,DIOStatusLast->nbDIOReceived,DIOStatusLast->nbDIOSuppressed);
                sprintf(buf2,"Joined!\nVerNum = %d\nRank = %d\nPrf.Parent = %s\nnbDIOSent = %d\nnbDIOReceived = %d\nnbDIOSuppressed = %d",VersionNember,Rank,PrParent.getSuffix(96).str().c_str(),DIOStatusLast->nbDIOSent,DIOStatusLast->nbDIOReceived,DIOStatusLast->nbDIOSuppressed);
                //EXTRA END
                host->getDisplayString().setTagArg("t", 0, buf2);
                cancelAndDelete(DIOTimer);
                DIOTimer = new cMessage("DIO-timer", SEND_DIO_TIMER);
                scheduleNextDIOTransmission();
                return;
            }
            else
            {
                if((DIO_c<DIORedun)&&(Version-1==VersionNember))
                {
                    cancelAndDelete(DIOTimer);
                    DIOTimer = new cMessage("DIO-timer", SEND_DIO_TIMER);
                    scheduleNextDIOTransmission();
                    return;
               }
            }
        }
    }
    else
        if(msg->getKind() == Global_REPAIR_TIMER)
        {
            DeleteDIOTimer();
            ScheduleNextGlobalRepair();
            if (DISEnable)
                for (int i=0; i<NodesNumber;i++)
                    if(i != pManagerRPL->getIndexFromAddress(sinkAddress)) //if(i!=sinkAddress) //EXTRA
                    {
                        NodesAddress[i]->SetDISParameters();
                        NodesAddress[i]->scheduleNextDISTransmission();
                    }
            scheduleNextDIOTransmission();
            return;
        }
            else
                if(msg->getKind() == SEND_DIS_FLOOD_TIMER)
                {
                    //if(((!IsNodeJoined[myNetwAddr])&&((DIS_c<DISRedun)||(DISRedun==0)))&&(DISVersion==Version))    //EXTRA
                    if(((!IsJoined)&&((DIS_c<DISRedun)||(DISRedun==0)))&&(DISVersion==Version))
                    {
                        //EXTRA BEGIN
                        ICMPv6DISMsg* pkt = new ICMPv6DISMsg("DIS", DIS_FLOOD);  //DISMessage* pkt = new DISMessage("DIS", DIS_FLOOD);  //EXTRA
                        IPv6ControlInfo *controlInfo = new IPv6ControlInfo;
                        controlInfo->setSrcAddr(myNetwAddr);
                        controlInfo->setDestAddr(IPv6Address::ALL_NODES_2); // IPv6Address::ALL_NODES_2 is ff02::2 (scope 2 (link-local)), FIXME: ff02::1a for rpl
                        //pkt->setInitialSrcAddr(myNetwAddr);
                        //pkt->setFinalDestAddr(LAddress::L3BROADCAST);
                        //pkt->setSrcAddr(myNetwAddr);
                        //pkt->setDestAddr(LAddress::L3BROADCAST);
                        //EXTRA END
                        pkt->setByteLength(DISheaderLength);
                        pkt->setVersionNumber(VersionNember);
                        pkt->setDODAGID(DODAGID);
                        //EXTRA BEGIN
                        //setDownControlInfo(pkt, LAddress::L2BROADCAST);
                        //sendDown(pkt);
                        pkt->setType(ICMPv6_RPL_CONTROL_MESSAGE);
                        controlInfo->setProtocol(IP_PROT_IPv6_ICMP);
                        pkt->setControlInfo(controlInfo);
                        send(pkt, icmpv6OutGateId);
                        //EXTRA END

                        if ((NodeCounter[Version]<NodesNumber)&&(!IsDODAGFormed))  NodeStateLast->DIS.Sent++;
                        DISStatusLast->nbDISSent++;
                        cancelAndDelete(DISTimer);
                        DISTimer = new cMessage("DIS-timer", SEND_DIS_FLOOD_TIMER);
                        scheduleNextDISTransmission();
                        return;
                    }
                    else
                        //if(((!IsNodeJoined[myNetwAddr])&&(DIS_c>=DISRedun))&&(DISVersion==Version))  //EXTRA
                        if(((!IsJoined)&&(DIS_c>=DISRedun))&&(DISVersion==Version))

                        {
                            if ((NodeCounter[Version]<NodesNumber)&&(!IsDODAGFormed))  NodeStateLast->DIS.Suppressed++;

                            DISStatusLast->nbDISSuppressed++;
                            char buf1[100];
                            sprintf(buf1, "DIS transmission suppressed!");
                            //cModule* host=findHost();    //EXTRA; host was defined in initiat method
                            host->bubble(buf1);
                            delete msg;
                            cancelAndDelete(DISTimer);
                            DISTimer = new cMessage("DIS-timer", SEND_DIS_FLOOD_TIMER);
                            scheduleNextDISTransmission();
                            return;

                        }
                        else
                            if(IsJoined) //if(IsNodeJoined[myNetwAddr])  //EXTRA
                            {
                                cancelAndDelete(DISTimer);
                                DISTimer = new cMessage("DIS-timer", SEND_DIS_FLOOD_TIMER);
                            }
                }
                else delete msg;
}


void RPLRouting::handleIncommingMessage(cMessage* msg)  //void RPLRouting::handleLowerMsg(cMessage* msg)  //EXTRA
{
    //EXTRA BEGIN
    EV << "->RPLRouting::handleIncommingMessage()" << endl;
    //SimpleBattery* battery;
    //battery = FindModule<SimpleBattery*>::findSubModule(findHost());
    //const cObject* pCtrlInfo = NULL;
    //cModule* host=findHost();  //EXTRA host has been initialized in the initialize method

    IPv6ControlInfo *ctrlInfo = nullptr;
    EV << "ICMPv6 message is received from ICMPv6 module, message name is " << msg->getName() << ", message kind is " << msg->getKind() << endl;
    //EXTRA END

    if(msg->getKind()==DIO)
    {
        //EXTRA BEGIN
        ICMPv6DIOMsg* netwMsg = check_and_cast<ICMPv6DIOMsg*>(msg);
        ctrlInfo = check_and_cast<IPv6ControlInfo *>(netwMsg->removeControlInfo());
        EV << "Received message is ICMPv6 DIO message, DODAGID address is " << netwMsg->getDODAGID() << ", src address is " << ctrlInfo->getSrcAddr() << endl;

        //pCtrlInfo = netwMsg->removeControlInfo();
        //EXTRA END

       if ((NodeCounter[Version]<NodesNumber)&&(!IsDODAGFormed)) NodeStateLast->DIO.Received++;
       if(myNetwAddr==sinkAddress)
        {
            DIOStatusLast->nbDIOReceived++;
            char buf2[100];
            sprintf(buf2,"DODAG ROOT\nVerNum = %d\nRank = %d\nnbDIOSent = %d\nnbDIOReceived = %d\nnbDIOSuppressed = %d",VersionNember,Rank,DIOStatusLast->nbDIOSent,DIOStatusLast->nbDIOReceived,DIOStatusLast->nbDIOSuppressed);
            host->getDisplayString().setTagArg("t", 0, buf2);
            char buf3[50];
            sprintf(buf3,"DIO  deleted!");
            host->bubble(buf3);
            delete msg;
            return;
        }
        else
        {
           if(!IsJoined)
            {
               IsJoined=true;
               IsNodeJoined[pManagerRPL->getIndexFromAddress(myNetwAddr)] = true;  //IsNodeJoined[myNetwAddr] = true;  //EXTRA
               VersionNember=netwMsg->getVersionNumber();

               DIOStatusNew = CreateNewVersionDIO();
               DIOStatusNew->nbDIOReceived++;
               if(DIOStatusHeader == NULL)
               {
                   DIOStatusLast = DIOStatusNew;
                   DIOStatusHeader = DIOStatusNew;
               }
               else
               {
                   DIOStatusLast->link = DIOStatusNew;
                   DIOStatusLast = DIOStatusNew;
               }

               NodeCounter[VersionNember]++;
               EV << "NodeCounter[" << VersionNember << "] = " << NodeCounter[VersionNember] << endl;  //EXTRA

               DODAGJoinTimeNew = CreateNewVersionJoiningTime();
               DODAGJoinTimeNew->TimetoJoinDODAG=netwMsg->getArrivalTime();
               if (DODAGJoinTimeHeader==NULL)
               {
                   DODAGJoinTimeLast = DODAGJoinTimeNew;
                   DODAGJoinTimeHeader = DODAGJoinTimeNew;
               }
               else
               {
                   DODAGJoinTimeLast->link = DODAGJoinTimeNew;
                   DODAGJoinTimeLast = DODAGJoinTimeNew;

               }

               //EXTRA BEGIN
               //NodeStateLast->JoiningDODAGTime[myNetwAddr] = DODAGJoinTimeLast->TimetoJoinDODAG;
               NodeStateLast->JoiningDODAGTime[pManagerRPL->getIndexFromAddress(myNetwAddr)] = DODAGJoinTimeLast->TimetoJoinDODAG;
               //EXTRA END

               DIO_CurIntsizeNext=DIOIntMin;
               DIO_StofCurIntNext=DODAGJoinTimeLast->TimetoJoinDODAG;
               DIO_EndofCurIntNext=DIO_StofCurIntNext+DIO_CurIntsizeNext;

               Grounded=netwMsg->getGrounded();
               DIOIntDoubl=netwMsg->getNofDoub();
               DIOIntMin=netwMsg->getIMin();
               DIORedun=netwMsg->getK();
               DODAGID=netwMsg->getDODAGID();
               //EXTRA BEGIN
               //AddParent(netwMsg->getSrcAddr(),netwMsg->getRank());
               AddParent(ctrlInfo->getSrcAddr(),netwMsg->getRank());
               //NodeStateNew->Rank[myNetwAddr] = Rank;
               NodeStateNew->Rank[pManagerRPL->getIndexFromAddress(myNetwAddr)] = Rank;
               //EXTRA END

               char buf0[50];
               //EXTRA BEGIN
               //sprintf(buf0, "I joined DODAG%d via node %d !!", VersionNember,int(netwMsg->getSrcAddr()));
               sprintf(buf0, "I joined DODAG%d via node %d !!", VersionNember,ctrlInfo->getSrcAddr());
               //EXTRA END
               host->bubble(buf0);
               char buf1[100];
               //EXTRA BEGIN
               //sprintf(buf1,"Joined!\nVerNum = %d\nRank = %d\nPrf.Parent = %d\nnbDIOSent = %d\nnbDIOReceived = %d\nnbDIOSuppressed = %d",VersionNember,Rank,int(PrParent),DIOStatusLast->nbDIOSent,DIOStatusLast->nbDIOReceived,DIOStatusLast->nbDIOSuppressed);
               sprintf(buf1,"Joined!\nVerNum = %d\nRank = %d\nPrf.Parent = %s\nnbDIOSent = %d\nnbDIOReceived = %d\nnbDIOSuppressed = %d",VersionNember,Rank,PrParent.getSuffix(96).str().c_str(),DIOStatusLast->nbDIOSent,DIOStatusLast->nbDIOReceived,DIOStatusLast->nbDIOSuppressed);
               //EXTRA END
               host->getDisplayString().setTagArg("t", 0, buf1);
               cancelAndDelete(DISTimer);
               DISTimer = new cMessage("DIS-timer", SEND_DIS_FLOOD_TIMER);
               scheduleNextDIOTransmission();
            }else
                if(netwMsg->getVersionNumber()>VersionNember)
                {
                    //IsNodeJoined[myNetwAddr] = true; //EXTRA
                    IsJoined = true;  //EXTRA
                    DeleteDIOTimer();
                    VersionNember=netwMsg->getVersionNumber();

                    DIOStatusNew = CreateNewVersionDIO();
                    DIOStatusNew->nbDIOReceived++;

                    if(DIOStatusHeader == NULL)
                    {
                        DIOStatusLast = DIOStatusNew;
                        DIOStatusHeader = DIOStatusNew;
                    }
                    else
                    {
                        DIOStatusLast->link = DIOStatusNew;
                        DIOStatusLast = DIOStatusNew;
                    }

                    NodeCounter[VersionNember]++;
                    EV << "NodeCounter[" << VersionNember << "] = " << NodeCounter[VersionNember] << endl;  //EXTRA

                    DODAGJoinTimeNew = CreateNewVersionJoiningTime();
                    DODAGJoinTimeNew->TimetoJoinDODAG=netwMsg->getArrivalTime();
                    if (DODAGJoinTimeHeader==NULL)
                    {
                        DODAGJoinTimeLast = DODAGJoinTimeNew;
                        DODAGJoinTimeHeader = DODAGJoinTimeNew;
                    }
                    else
                    {
                        DODAGJoinTimeLast->link = DODAGJoinTimeNew;
                        DODAGJoinTimeLast = DODAGJoinTimeNew;
                    }
                    //EXTRA BEGIN
                    //NodeStateLast->JoiningDODAGTime[myNetwAddr] = DODAGJoinTimeLast->TimetoJoinDODAG;
                    NodeStateLast->JoiningDODAGTime[pManagerRPL->getIndexFromAddress(myNetwAddr)] = DODAGJoinTimeLast->TimetoJoinDODAG;
                    //EXTRA END
                    DIOIntDoubl=netwMsg->getNofDoub();
                    DIOIntMin=netwMsg->getIMin();
                    DIORedun=netwMsg->getK();
                    DODAGID=netwMsg->getDODAGID();
                    DIO_CurIntsizeNext=DIOIntMin;
                    DIO_StofCurIntNext=DODAGJoinTimeLast->TimetoJoinDODAG;
                    DIO_EndofCurIntNext=DIO_StofCurIntNext+DIO_CurIntsizeNext;
                    Grounded=netwMsg->getGrounded();
                    //EXTRA BEGIN
                    //AddParent(netwMsg->getSrcAddr(),netwMsg->getRank());
                    AddParent(ctrlInfo->getSrcAddr(),netwMsg->getRank());
                    //NodeStateNew->Rank[myNetwAddr] = Rank;
                    NodeStateNew->Rank[pManagerRPL->getIndexFromAddress(myNetwAddr)] = Rank;
                    //EXTRA END

                    char buf0[50];
                    //EXTRA BEGIN
                    //sprintf(buf0, "I joined DODAG %d via node %d !!", VersionNember,int(netwMsg->getSrcAddr()));
                    sprintf(buf0, "I joined DODAG %d via node %d !!", VersionNember,ctrlInfo->getSrcAddr());
                    //EXTRA END
                    host->bubble(buf0);
                    char buf1[100];
                    //EXTRA BEGIN
                    //sprintf(buf1,"Joined!\nVerNum = %d\nRank = %d\nPrf.Parent = %d\nnbDIOSent = %d\nnbDIOReceived = %d\nnbDIOSuppressed = %d",VersionNember,Rank,int(PrParent),DIOStatusLast->nbDIOSent,DIOStatusLast->nbDIOReceived,DIOStatusLast->nbDIOSuppressed);
                    sprintf(buf1,"Joined!\nVerNum = %d\nRank = %d\nPrf.Parent = %s\nnbDIOSent = %d\nnbDIOReceived = %d\nnbDIOSuppressed = %d",VersionNember,Rank,PrParent.getSuffix(96).str().c_str() ,DIOStatusLast->nbDIOSent,DIOStatusLast->nbDIOReceived,DIOStatusLast->nbDIOSuppressed);
                    //EXTRA END
                    host->getDisplayString().setTagArg("t", 0, buf1);
                    cancelAndDelete(DISTimer);
                    DISTimer = new cMessage("DIS-timer", SEND_DIS_FLOOD_TIMER);
                    scheduleNextDIOTransmission();

                }else
                if((netwMsg->getRank()<=Rank)&&(netwMsg->getVersionNumber()==VersionNember))
                {
                    DIOStatusLast->nbDIOReceived++;
                    DIO_c++;
                    DODAGID=netwMsg->getDODAGID();
                    Grounded=netwMsg->getGrounded();
                    DIOIntDoubl=netwMsg->getNofDoub();
                    DIOIntMin=netwMsg->getIMin();
                    DIORedun=netwMsg->getK();
                    switch(IsParent(ctrlInfo->getSrcAddr(),netwMsg->getRank()))  //switch(IsParent(netwMsg->getSrcAddr(),netwMsg->getRank()))  //EXTRA
                    {
                        case NOT_EXIST:
                            AddParent(ctrlInfo->getSrcAddr(),netwMsg->getRank());  //AddParent(netwMsg->getSrcAddr(),netwMsg->getRank());  //EXTRA
                            break;
                        case SHOULD_BE_UPDATED:
                            DeleteParent(ctrlInfo->getSrcAddr());  //DeleteParent(netwMsg->getSrcAddr());  //EXTRA
                            AddParent(ctrlInfo->getSrcAddr(),netwMsg->getRank());  //AddParent(netwMsg->getSrcAddr(),netwMsg->getRank());  //EXTRA
                            break;
                        case EXIST:
                            break;
                    }
                    char buf2[255];
                    //EXTRA BEGIN
                    //sprintf(buf2, "A DIO received from node %d !", int(netwMsg->getSrcAddr()));
                    sprintf(buf2, "A DIO received from node %d !", ctrlInfo->getSrcAddr());
                    //EXTRA END
                    host->bubble(buf2);
                    char buf3[100];
                    //EXTRA BEGIN
                    //sprintf(buf3,"Joined!\nVerNum = %d\nRank = %d\nPrf.Parent = %d\nnbDIOSent = %d\nnbDIOReceived = %d\nnbDIOSuppressed = %d",VersionNember,Rank,int(PrParent),DIOStatusLast->nbDIOSent,DIOStatusLast->nbDIOReceived,DIOStatusLast->nbDIOSuppressed);
                    sprintf(buf3,"Joined!\nVerNum = %d\nRank = %d\nPrf.Parent = %s\nnbDIOSent = %d\nnbDIOReceived = %d\nnbDIOSuppressed = %d",VersionNember,Rank,PrParent.getSuffix(96).str().c_str() ,DIOStatusLast->nbDIOSent,DIOStatusLast->nbDIOReceived,DIOStatusLast->nbDIOSuppressed);
                    //EXTRA END
                    host->getDisplayString().setTagArg("t", 0, buf3);
                }
                else
                    if(netwMsg->getVersionNumber()<VersionNember)
                    {
                        // The sender of this DIO should be updated.
                        host->bubble("DIO deleted!!\nThe sender node should be updated.!!! ");
                        //delete msg;  //EXTRA because msg is deleted bellow
                    }
                    else
                    {
                        DIOStatusLast->nbDIOReceived++;
                        char buf4[100];
                        //EXTRA BEGIN
                        //sprintf(buf4,"Joined!\nVerNum = %d\nRank = %d\nPrf.Parent = %d\nnbDIOSent = %d\nnbDIOReceived = %d\nnbDIOSuppressed = %d",VersionNember,Rank,int(PrParent),DIOStatusLast->nbDIOSent,DIOStatusLast->nbDIOReceived,DIOStatusLast->nbDIOSuppressed);
                        sprintf(buf4,"Joined!\nVerNum = %d\nRank = %d\nPrf.Parent = %s\nnbDIOSent = %d\nnbDIOReceived = %d\nnbDIOSuppressed = %d",VersionNember,Rank,PrParent.getSuffix(96).str().c_str(),DIOStatusLast->nbDIOSent,DIOStatusLast->nbDIOReceived,DIOStatusLast->nbDIOSuppressed);
                        //EXTRA END
                        host->getDisplayString().setTagArg("t", 0, buf4);
                        host->bubble("DIO deleted!!");
                        //delete msg;  //EXTRA because msg is deleted below
                    }
            if((NodeCounter[Version]==NodesNumber)&&(!IsDODAGFormed))
            {
                FileRecordCounter++;
                host->bubble("I'm the last node that joined DODAG! DODAG formed!!");
                IsDODAGFormed=true;
                NodeStateLast->DODAGsFormationTimeRecords = netwMsg->getArrivalTime()-DODAGSartTime;
                AvgDODAGFomationTime+=NodeStateLast->DODAGsFormationTimeRecords;
                AvgAllDIOsSent+=NodeStateLast->DIO.Sent;
                AvgAllDIOsReceived+=NodeStateLast->DIO.Received;
                AvgAllDIOsSuppressed+=NodeStateLast->DIO.Suppressed;
                NodeCounter[Version]++;
                EV << "NodeCounter[" << VersionNember << "] = " << NodeCounter[VersionNember] << endl;  //EXTRA
//                EV<<"\n\n\n\nDODAG["<<VersionNember<<"] formed in "<<NodeStateLast->DODAGsFormationTimeRecords<<"s. \n\n\n\n";
                //EXTRA BEGIN
                //NodeStateLast->Collision = GetnbFramesWithInterferenceDropped();
                //NodeStateLast->PacketLost= GetnbFramesWithoutInterferenceDropped();
                //AvgAllCollisionNarmal +=NodeStateLast->Collision;

                //for (int i=0; i<NodesNumber;i++)
                    //NodeStateLast->PowerConsumption[i] = battery->CalculateRemainenergy(i)-NodeStateLast->PowerConsumption[i] ;

                NofDODAGformationNormal++;
                EV << "Number of DODAGformationNormal is " << NofDODAGformationNormal << endl;  //EXTRA

                if(NodeStateLast->DODAGsFormationTimeRecords!=0)
                {
                    FileRecord.Collosion[FileRecordCounter] = NodeStateLast->Collision;
                    FileRecord.FormationTime[FileRecordCounter] = SIMTIME_DBL(NodeStateLast->DODAGsFormationTimeRecords);
                    FileRecord.DIOSent[FileRecordCounter] = NodeStateLast->DIO.Sent;
                    FileRecord.DISSent[FileRecordCounter] = NodeStateLast->DIS.Sent;
                    FileRecord.PacketLost[FileRecordCounter] = NodeStateLast->PacketLost;
                    //EXTRA BEGIN
                    FileRecord.numParents[FileRecordCounter] = NodeStateLast->numParents;
                    FileRecord.numPreferedParents[FileRecordCounter] = NodeStateLast->numPreferedParents;
                    //EXTRA END

                }
                if(NodeStateNew->DODAGsFormationTimeRecords!=0)
                {
                    for (int i=0; i<NodesNumber;i++)
                    {
                        if(i!=pManagerRPL->getIndexFromAddress(sinkAddress))  //if(i!=sinkAddress)  //EXTRA
                        {
                            //EXTRA BEGIN
                            //FileRecord.OtherFields[FileRecordCounter].JoiningTime[i] = SIMTIME_DBL(NodeStateLast->JoiningDODAGTime[i] - NodeStateLast->JoiningDODAGTime[sinkAddress]);
                            FileRecord.OtherFields[FileRecordCounter].JoiningTime[i] = SIMTIME_DBL(NodeStateLast->JoiningDODAGTime[i] - NodeStateLast->JoiningDODAGTime[pManagerRPL->getIndexFromAddress(sinkAddress)]);
                            //EXTRA END
                            FileRecord.OtherFields[FileRecordCounter].NodesRank[i] = NodeStateLast->Rank[i];
                        }
                        FileRecord.OtherFields[FileRecordCounter].ConsumedPower[i] = NodeStateLast->PowerConsumption[i];
                    }
                }
                if (GlobalRepairTimer!=0)
                    //EXTRA BEGIN
                    //NodesAddress[sinkAddress]->DeleteScheduledNextGlobalRepair();
                    NodesAddress[pManagerRPL->getIndexFromAddress(sinkAddress)]->DeleteScheduledNextGlobalRepair();
                    //EXTRA END
                else
                    //EXTRA BEGIN
                    //Datasaving(sinkAddress, DISEnable);
                    Datasaving(pManagerRPL->getIndexFromAddress(sinkAddress),DISEnable);
                //EXTRA END
            }
            delete netwMsg;
            if(GRT_Counter==NumberofIterations)
            {
                //EXTRA BEGIN
                //Datasaving(sinkAddress,DISEnable);
                Datasaving(pManagerRPL->getIndexFromAddress(sinkAddress), DISEnable);
                //EXTRA END
                endSimulation();
            }
        }
    }
    else
        //if ((msg->getKind() == DIS_FLOOD)&&(IsNodeJoined[myNetwAddr]))  //EXTRA
        if ((msg->getKind() == DIS_FLOOD)&&(IsJoined))
        {
            DIS_c++;
            NodeStateLast->DIS.Received++;
            ICMPv6DISMsg *netwMsg = check_and_cast<ICMPv6DISMsg *>(msg);  //DISMessage *netwMsg = check_and_cast<DISMessage *>(msg); //EXTRA
            ctrlInfo = check_and_cast<IPv6ControlInfo *> (netwMsg->removeControlInfo());  //pCtrlInfo = netwMsg->removeControlInfo();  //EXTRA
            char buf2[255];
            //EXTRA BEGIN
            //sprintf(buf2, "A DIS message received from node %d!\nResetting Trickle timer!", int(netwMsg->getSrcAddr()));
            sprintf(buf2, "A DIS message received from node %d!\nResetting Trickle timer!", ctrlInfo->getSrcAddr());
            //EXTRA END
            host->bubble(buf2);
            TrickleReset();
            delete netwMsg;
            scheduleNextDIOTransmission();
        }
        else
            //if ((msg->getKind() == DIS_FLOOD)&&(!IsNodeJoined[myNetwAddr]))  //EXTRA
            if ((msg->getKind() == DIS_FLOOD)&&(!IsJoined))
            {
                DIS_c++;
                NodeStateLast->DIS.Received++;
                ICMPv6DISMsg* netwMsg = check_and_cast<ICMPv6DISMsg*>(msg);  //DISMessage* netwMsg = check_and_cast<DISMessage*>(msg);  //EXTRA
                ctrlInfo = check_and_cast<IPv6ControlInfo *>(netwMsg->removeControlInfo());  //pCtrlInfo = netwMsg->removeControlInfo();  //EXTRA
                char buf2[255];
                //EXTRA
                //sprintf(buf2, "A DIS message received from node %d!\nBut I am not a member of any DODAG!", int(netwMsg->getSrcAddr()));
                sprintf(buf2, "A DIS message received from node %s!\nBut I am not a member of any DODAG!", ctrlInfo->getSrcAddr());
                //EXTRA END
                host->bubble(buf2);
                delete netwMsg;
            }
            else delete msg;
   //EXTRA BEGIN
    //if (pCtrlInfo != NULL)
        //delete pCtrlInfo;
    if (ctrlInfo != NULL)
        delete ctrlInfo;
    EV << "<-RPLRouting::handleIncommingMessage()" << endl;
    //EXTRA END
}


void RPLRouting::handleLowerControl(cMessage *msg)
{
    delete msg;
}

void RPLRouting::handleUpperMsg(cMessage* msg)
{
}

void RPLRouting::finish()
{

}

/*cMessage* RPLRouting::decapsMsg(ICMPv6DIOMsg *msg)  //EXTRA
{
    cMessage *m = msg->decapsulate();
    setUpControlInfo(m, msg->getSrcAddr());
    delete msg;
    return m;
}
*/

int RPLRouting::IsParent(const IPv6Address& id,int idrank)
{
    for(int i=0;i<NofParents[VersionNember];i++)
      if (Parents[VersionNember][i].ParentId==id)
      {
          if (Parents[VersionNember][i].ParentRank==idrank)
           return(EXIST);
         else
           return(SHOULD_BE_UPDATED);
      }
    return(NOT_EXIST);
}

void RPLRouting::AddParent(const IPv6Address& id,int idrank)
{
    //EXTRA BEGIN
    if(!hasRoute[VersionNember]){
        hasRoute[VersionNember] = true;
        NodeStateLast->numPreferedParents++; // next condition is not a good place for this statement because when deleteParent() increaments NofParents[VersionNember], the condition is true, and numPreferedParents increaments twice or more
    }
    //EXTRA END
    if(NofParents[VersionNember]==0)
    {
        Parents[VersionNember][0].ParentId=id;
        Parents[VersionNember][0].ParentRank=idrank;
        PrParent=Parents[VersionNember][0].ParentId;
        Rank=Parents[VersionNember][0].ParentRank+1;
        NofParents[VersionNember]++;
        NodeStateLast->numParents++;     //EXTRA
        return;
    }else
    {
        if (NofParents[VersionNember]==MaxNofParents)
        {
           if (idrank >= Parents[VersionNember][NofParents[VersionNember]-1].ParentRank) return;
           //EXTRA BEGIN
           //else NofParents[VersionNember]--;
           else{
               NofParents[VersionNember]--;
               NodeStateLast->numParents--;
           }
           //EXTRA END
        }
        int i=NofParents[VersionNember]-1;
        while((i>=0)&&(idrank<Parents[VersionNember][i].ParentRank))
        {
            Parents[VersionNember][i+1]=Parents[VersionNember][i];
            i--;
        }
        Parents[VersionNember][i+1].ParentId=id;
        Parents[VersionNember][i+1].ParentRank=idrank;
        PrParent=Parents[VersionNember][0].ParentId;
        Rank=Parents[VersionNember][0].ParentRank+1;
        NofParents[VersionNember]++;
        NodeStateLast->numParents++;     //EXTRA
    }
    return;
}
void RPLRouting::DeleteParent(const IPv6Address& id)
{
    for(int i=0;i<NofParents[VersionNember];i++)
        if (Parents[VersionNember][i].ParentId==id)
        {
            for(int j=i;j<NofParents[VersionNember]-1;j++)
                Parents[VersionNember][j]=Parents[VersionNember][j+1];
            break;
        }
    NofParents[VersionNember]--;
    NodeStateLast->numParents--;  //EXTRA
}

RPLRouting::DIOStatus* RPLRouting::CreateNewVersionDIO()
{
    DIOStatus* Temp;
    Temp = new DIOStatus;
    Temp->version=VersionNember;
    Temp->nbDIOSent=0;
    Temp->nbDIOReceived=0;
    Temp->nbDIOSuppressed=0;
    Temp->link=NULL;
    return Temp;
}

RPLRouting::DISStatus* RPLRouting::CreateNewVersionDIS()
{
    DISStatus* Temp;
    Temp = new DISStatus;
    Temp->nbDISSent=0;
    Temp->nbDISReceived=0;
    Temp->nbDISSuppressed=0;
    Temp->link=NULL;
    return Temp;
}
void RPLRouting::DISHandler()
{
    Enter_Method("DISHandler()");
    DISVersion = Version;
    DISStatusNew = CreateNewVersionDIS();
    if(DISStatusHeader == NULL)
    {
        DISStatusLast = DISStatusNew;
        DISStatusHeader = DISStatusNew;
    }
    else
    {
        DISStatusLast->link = DISStatusNew;
        DISStatusLast = DISStatusNew;
    }
}

RPLRouting::DODAGJoiningtime* RPLRouting::CreateNewVersionJoiningTime()
{
    DODAGJoiningtime* Temp;
    Temp = new DODAGJoiningtime;
    Temp->version=VersionNember;
    Temp->link=NULL;
    return Temp;
}

//EXTRA BEGIN
bool RPLRouting::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
/*
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_LINK_LAYER) {
            start();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_LINK_LAYER) {
            stop();
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH) {
            stop();
        }
    }
    else {
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    }

    return true;
    */
}
//EXTRA END
RPLRouting::~RPLRouting()
{
    cancelAndDelete(DIOTimer);
    cancelAndDelete(DISTimer);
    cancelAndDelete(GRepairTimer);
    delete [] NofParents;
    for( int i = 0 ; i < NumberofIterations+1 ; i++ )
        delete [] Parents[i];
    delete [] Parents;

    DIOStatusNew  = DIOStatusHeader;
    while(DIOStatusNew)
    {
        DIOStatusNew= DIOStatusNew->link;
        delete DIOStatusHeader;
        DIOStatusHeader = DIOStatusNew;
    }

    DISStatusNew  = DISStatusHeader;
    while(DISStatusNew)
    {
        DISStatusNew= DISStatusNew->link;
        delete DISStatusHeader;
        DISStatusHeader = DISStatusNew;
    }

    DODAGJoinTimeNew  = DODAGJoinTimeHeader;
    while(DIOStatusNew)
    {
        DODAGJoinTimeNew= DODAGJoinTimeNew->link;
        delete DODAGJoinTimeHeader;
        DODAGJoinTimeHeader = DODAGJoinTimeNew;
    }

//EXTRA BEGIN
/*    if (pManagerRPL->getIndexFromAddress(myNetwAddr) == NodesNumber - 1) //if (myNetwAddr==NodesNumber-1)  //EXTRA
    {
        delete [] NodeCounter;
        FileRecordMemoryDeallocation();
        NodeStateNew = NodeStateHeader;
        while(NodeStateNew)
        {
            NodeStateNew= NodeStateHeader->Link;

            delete [] NodeStateHeader->Rank;
            delete [] NodeStateHeader->JoiningDODAGTime;
            delete [] NodeStateHeader->PowerConsumption;
            delete NodeStateHeader;
            NodeStateHeader = NodeStateNew;
        }
    }
    */
    if (NodeCounter){
        delete [] NodeCounter;
        NodeCounter = nullptr;    //for mutual exclusion
    }

    FileRecordMemoryDeallocation();

    NodeStateNew = NodeStateHeader;
    while(NodeStateNew)
    {
        NodeStateNew= NodeStateHeader->Link;

        delete [] NodeStateHeader->Rank;
        delete [] NodeStateHeader->JoiningDODAGTime;
        delete [] NodeStateHeader->PowerConsumption;
        delete NodeStateHeader;
        NodeStateHeader = NodeStateNew;
    }
    //EXTRA END



}

void Datasaving(int sinkAddressIndex, bool DISEnable)  //void Datasaving(int sinkAddress, bool DISEnable) //EXTRA
{
    FileRecord.IterationsNumber = NofDODAGformationNormal;
    char * temp = SetPath(MainPath,"IterationsNumber_K",K_value);  //EXTRA
    strcpy(Path, temp);  //strcpy(Path,SetPath(MainPath,"IterationsNumber_K",K_value)); //EXTRA
    delete[] temp;  //EXTRA
    IterationsNumber = fopen(Path,Mode);
    fprintf(IterationsNumber,"%d\n",NofDODAGformationNormal);


    temp = SetPath(MainPath,"JoiningTime_K",K_value);  //EXTRA
    strcpy(Path, temp);  //strcpy(Path,SetPath(MainPath,"JoiningTime_K",K_value));  //EXTRA
    delete[] temp;  //EXTRA
    JoiningTime = fopen(Path,Mode);

    //EXTRA BEGIN
    /*
    temp = SetPath(MainPath,"Collosion_K",K_value);  //EXTRA
    strcpy(Path, temp);  //strcpy(Path,SetPath(MainPath,"Collosion_K",K_value));  //EXTRA
    delete[] temp;  //EXTRA
    Collosion = fopen(Path,Mode);
    */
    //EXTRA

    temp = SetPath(MainPath,"DIOSent_K",K_value);  //EXTRA
    strcpy(Path, temp);  //strcpy(Path,SetPath(MainPath,"DIOSent_K",K_value));  //EXTRA
    delete[] temp;  //EXTRA
    DIOSent = fopen(Path,Mode);

    temp = SetPath(MainPath,"FormationTime_K",K_value);  //EXTRA
    strcpy(Path, temp);  //strcpy(Path,SetPath(MainPath,"FormationTime_K",K_value));  //EXTRA
    delete[] temp;  //EXTRA
    FormationTime = fopen(Path,Mode);


    temp = SetPath(MainPath,"numTableEntris_K",K_value);  //EXTRA
    strcpy(Path, temp);  //strcpy(Path,SetPath(MainPath,"FormationTime_K",K_value));  //EXTRA
    delete[] temp;  //EXTRA
    numTableEntris = fopen(Path,Mode);

    //EXTRA BEGIN
    /*
    temp = SetPath(MainPath,"PacketErrors_K",K_value);  //EXTRA
    strcpy(Path, temp);  //strcpy(Path,SetPath(MainPath,"PacketErrors_K",K_value));  //EXTRA
    delete[] temp;  //EXTRA
    PacketLost = fopen(Path,Mode);
    */
    //EXTRA END

    temp = SetPath(MainPath,"NodesRank_K",K_value);  //EXTRA
    strcpy(Path, temp);  //strcpy(Path,SetPath(MainPath,"NodesRank_K",K_value));  //EXTRA
    delete[] temp;  //EXTRA
    NodesRank = fopen(Path,Mode);

    //EXTRA BEGIN
    temp = SetPath(MainPath,"numPreferedParent_K",K_value);  //EXTRA
    strcpy(Path, temp);  //strcpy(Path,SetPath(MainPath,"JoiningTime_K",K_value));  //EXTRA
    delete[] temp;  //EXTRA
    preferedParent = fopen(Path,Mode);

    temp = SetPath(MainPath,"numParents_K",K_value);  //EXTRA
    strcpy(Path, temp);  //strcpy(Path,SetPath(MainPath,"JoiningTime_K",K_value));  //EXTRA
    delete[] temp;  //EXTRA
    numberOfParents = fopen(Path,Mode);
    //EXTRA END

    //EXTRA BEGIN
    /*
    temp = SetPath(MainPath,"ConsumedPower_K",K_value);  //EXTRA
    strcpy(Path, temp);  //strcpy(Path,SetPath(MainPath,"ConsumedPower_K",K_value));  //EXTRA
    delete[] temp;  //EXTRA
    ConsumedPower = fopen(Path,Mode);
    */
    //EXTRA END

    if (DISEnable)
    {
        temp = SetPath(MainPath,"DISSent_K",K_value);  //EXTRA
        strcpy(Path, temp);  //strcpy(Path,SetPath(MainPath,"DISSent_K",K_value));  //EXTRA
        delete[] temp;  //EXTRA
        DISSent = fopen(Path,Mode);
        for(int j=0;j<NofDODAGformationNormal;j++)
            fprintf(DISSent,"%d\n",FileRecord.DISSent[j]);
    }

    for(int j=0;j<NofDODAGformationNormal;j++)
    {
        fprintf(FormationTime,"%f\n",FileRecord.FormationTime[j]);
        //fprintf(Collosion,"%d\n",FileRecord.Collosion[j]);  //EXTRA
        fprintf(DIOSent,"%d\n",FileRecord.DIOSent[j]);
        //fprintf(PacketLost,"%d\n",FileRecord.PacketLost[j]);  //EXTRA
        fprintf(preferedParent,"%d\n",FileRecord.numPreferedParents[j]);  //EXTRA
        fprintf(numberOfParents,"%d\n",FileRecord.numParents[j]);  //EXTRA
        fprintf(numTableEntris,"%d\n",FileRecord.numParents[j] + FileRecord.numPreferedParents[j]);  //EXTRA

        for (int i=0; i<NodesNumber;i++)
        {
            if(i != sinkAddressIndex) //if(i!=sinkAddress)  //EXTRA
            {
                fprintf(JoiningTime,"%f\n",FileRecord.OtherFields[j].JoiningTime[i]);
                fprintf(NodesRank,"%d\n",FileRecord.OtherFields[j].NodesRank[i]);
            }
            //fprintf(ConsumedPower,"%f\n",FileRecord.OtherFields[j].ConsumedPower[i]); //EXTRA
        }
    }

    fclose(JoiningTime);
    //fclose(Collosion);  //EXTRA
    fclose(DIOSent);
    fclose(DISSent);
    fclose(FormationTime);
    //fclose(PacketLost);  //EXTRA
    fclose(NodesRank);
    //fclose(ConsumedPower);  //EXTRA
    fclose(IterationsNumber);
    fclose(preferedParent);  //EXTRA
    fclose(numberOfParents);  //EXTRA

}

} // namespace rpl
