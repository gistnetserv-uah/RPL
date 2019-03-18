//
// Generated file, do not edit! Created by nedtool 5.2 from src/networklayer/icmpv6/ICMPv6MessageRPL.msg.
//

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#ifndef __INET_ICMPV6MESSAGERPL_M_H
#define __INET_ICMPV6MESSAGERPL_M_H

#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0502
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif

// cplusplus {{
#include "inet/common/INETDefs.h"
#include "inet/networklayer/icmpv6/ICMPv6Message_m.h"
#include "inet/networklayer/contract/ipv6/IPv6Address.h"
#define ICMPv6_HEADER_BYTES  8
// }}


namespace inet {

/**
 * Enum generated from <tt>src/networklayer/icmpv6/ICMPv6MessageRPL.msg:36</tt> by nedtool.
 * <pre>
 * enum ICMPv6TypeRPL
 * {
 * 
 *     ICMPv6_RPL_CONTROL_MESSAGE = 155; //RFC 6550, section 6, page 30
 * }
 * </pre>
 */
enum ICMPv6TypeRPL {
    ICMPv6_RPL_CONTROL_MESSAGE = 155
};

/**
 * Enum generated from <tt>src/networklayer/icmpv6/ICMPv6MessageRPL.msg:45</tt> by nedtool.
 * <pre>
 * //EXTRA
 * //values of code field
 * enum ICMPv6_RPL_CONTROL_MSG
 * {
 * 
 *     //     0x00: DODAG Information Solicitation (Section 6.2)
 *     DIS = 0x00;
 * 
 *     //     0x01: DODAG Information Object (Section 6.3)
 *     DIO = 0x01;
 * 
 *     //     0x02: Destination Advertisement Object (Section 6.4)
 *     DAO = 0x02;
 * //     0x03: Destination Advertisement Object Acknowledgment (Section 6.5)
 * 
 * //     0x80: Secure DODAG Information Solicitation (Section 6.2.2)
 * 
 * //     0x81: Secure DODAG Information Object (Section 6.3.2)
 * 
 * //     0x82: Secure Destination Advertisement Object (Section 6.4.2)
 * 
 * //     0x83: Secure Destination Advertisement Object Acknowledgment (Section 6.5.2)
 * 
 * //     0x8A: Consistency Check (Section 6.6)
 * 
 * }
 * 
 * //EXTRA 
 * //RPL Mode Of Operation
 * //enum RPLMOP{
 * //    No_Downward_Routes_maintained_by_RPL = 0;
 * //    Non_Storing_Mode_of_Operation = 1;
 * //    Storing_Mode_of_Operation_with_no_multicast_support = 2;
 * //    Storing_Mode_of_Operation_with_multicast_support = 3;
 * //}
 * </pre>
 */
enum ICMPv6_RPL_CONTROL_MSG {
    DIS = 0x00,
    DIO = 0x01,
    DAO = 0x02
};

/**
 * Enum generated from <tt>src/networklayer/icmpv6/ICMPv6MessageRPL.msg:83</tt> by nedtool.
 * <pre>
 * //EXTRA
 * //DIS options RFC 6550 Section 6.2.3
 * //DIO options RFC 6550 Section 6.3.3
 * //DAO options  RFC 6550 Section 6.4.3
 * enum RPL_OPTIONS
 * {
 * 
 *     PAD1 = 0x00; //DIS, DIO, and DAO
 *     PADN = 0x01; //DIS, DIO, and DAO
 * 
 *     Solicited_Information = 0x07;  //DIS
 * 
 *     DAG_Metric_Container = 0x02;   //DIO
 *     Routing_Information = 0x03;    //DIO
 *     DODAG_Configuration = 0x04;    //DIO
 *     Prefix_Information = 0x08;     //DIO
 * 
 *     RPL_Target = 0x05;             //DAO
 *     Transit_Information = 0x06;    //DAO
 *     RPL_Target_Descriptor = 0x09;  //DAO
 * 
 * 
 * }
 * 
 * //EXTRA
 * //6.2.1.  Format of the DIS Base Object
 * 
 * //        0                   1                   2
 * //        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
 * //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * //       |     Flags     |   Reserved    |   Option(s)...
 * //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * </pre>
 */
enum RPL_OPTIONS {
    PAD1 = 0x00,
    PADN = 0x01,
    Solicited_Information = 0x07,
    DAG_Metric_Container = 0x02,
    Routing_Information = 0x03,
    DODAG_Configuration = 0x04,
    Prefix_Information = 0x08,
    RPL_Target = 0x05,
    Transit_Information = 0x06,
    RPL_Target_Descriptor = 0x09
};

/**
 * Class generated from <tt>src/networklayer/icmpv6/ICMPv6MessageRPL.msg:113</tt> by nedtool.
 * <pre>
 * //DIS control message for RPL
 * //message ICMPv6DISMsg extends ICMPv6Message
 * packet ICMPv6DISMsg extends ICMPv6Message
 * {
 *     int code \@enum(ICMPv6_RPL_CONTROL_MSG); // RFC 6550, section 6: set to 0x00
 *     // TODO: checksum 
 *     int flags; // RFC 6550, section 6.2.1: set to 0
 *     int reserved; // RFC 6550, section 6.2.1 set to 0
 * 
 *     int RPLInstanceID;          // The ID of the RPL instance
 *     int VersionNumber;          // DODAG version number
 *     int V;                      // Node's rank
 *     int I;                      // Type of the DODAG, Grounded or Flooding
 *     int D;                      // Destination Advertisement Trigger Sequence Number       
 *     int Flag;                   // The size of Imin in Trcikle algorithm
 *     IPv6Address DODAGID;   // IPv6 address set by DODAG root  
 * 
 *     int options \@enum(RPL_OPTIONS); //RFC 6550, section 6.2.1
 * }
 * 
 * 
 * 
 * //EXTRA 
 * //6.3.1 Format of the DIO Base Object
 * 
 * //        0                   1                   2                   3
 * //        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * //       | RPLInstanceID |Version Number |             Rank              |
 * //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * //       |G|0| MOP | Prf |     DTSN      |     Flags     |   Reserved    |
 * //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * //       |                                                               |
 * //       +                                                               +
 * //       |                                                               |
 * //       +                            DODAGID                            +
 * //       |                                                               |
 * //       +                                                               +
 * //       |                                                               |
 * //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * //       |   Option(s)...
 * //       +-+-+-+-+-+-+-+-+
 * </pre>
 */
class ICMPv6DISMsg : public ::inet::ICMPv6Message
{
  protected:
    int code;
    int flags;
    int reserved;
    int RPLInstanceID;
    int VersionNumber;
    int V;
    int I;
    int D;
    int Flag;
    IPv6Address DODAGID;
    int options;

  private:
    void copy(const ICMPv6DISMsg& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const ICMPv6DISMsg&);

  public:
    ICMPv6DISMsg(const char *name=nullptr, short kind=0);
    ICMPv6DISMsg(const ICMPv6DISMsg& other);
    virtual ~ICMPv6DISMsg();
    ICMPv6DISMsg& operator=(const ICMPv6DISMsg& other);
    virtual ICMPv6DISMsg *dup() const override {return new ICMPv6DISMsg(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int getCode() const;
    virtual void setCode(int code);
    virtual int getFlags() const;
    virtual void setFlags(int flags);
    virtual int getReserved() const;
    virtual void setReserved(int reserved);
    virtual int getRPLInstanceID() const;
    virtual void setRPLInstanceID(int RPLInstanceID);
    virtual int getVersionNumber() const;
    virtual void setVersionNumber(int VersionNumber);
    virtual int getV() const;
    virtual void setV(int V);
    virtual int getI() const;
    virtual void setI(int I);
    virtual int getD() const;
    virtual void setD(int D);
    virtual int getFlag() const;
    virtual void setFlag(int Flag);
    virtual IPv6Address& getDODAGID();
    virtual const IPv6Address& getDODAGID() const {return const_cast<ICMPv6DISMsg*>(this)->getDODAGID();}
    virtual void setDODAGID(const IPv6Address& DODAGID);
    virtual int getOptions() const;
    virtual void setOptions(int options);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const ICMPv6DISMsg& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, ICMPv6DISMsg& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>src/networklayer/icmpv6/ICMPv6MessageRPL.msg:156</tt> by nedtool.
 * <pre>
 * //DIO control message for RPL
 * //message ICMPv6DIOMsg extends ICMPv6Message
 * packet ICMPv6DIOMsg extends ICMPv6Message
 * {
 *     int code \@enum(ICMPv6_RPL_CONTROL_MSG); // RFC 6550, section 6: set to 0x01
 *     // TODO: checksum 
 *     //int rplInstanceId = 0; // RFC 6550, section 6.3.1
 *     int versionNumber = 0; // RFC 6550, section 6.3.1
 *     int rank = 0;
 *     int grounded;
 *     //0
 *     //int MOP \@enum(RPLMOP);
 *     int MOP;
 *     //Prf
 *     //int flags = 0; // RFC 6550, section 6.3.1
 *     //int reserved = 0; // RFC 6550, section 6.3.1
 *     int DTSN;                 // Destination Advertisement Trigger Sequence Number       
 *     double IMin;              // The size of Imin in Trcikle algorithm
 *     int NofDoub;              // Number of doubling in Trcikle algorithm
 *     int k;                    // Redundancy constant in Trcikle algorithm
 *     IPv6Address DODAGID;              // IPv6 address set by DODAG root 
 * 
 * 
 * 
 *     int options \@enum(RPL_OPTIONS); //RFC 6550, section 6.3.1
 * }
 * 
 * 
 * 
 * //6.4.1 Format of the DAO Base Object
 * 
 * //        0                   1                   2                   3
 * //        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * //       | RPLInstanceID |K|D|   Flags   |   Reserved    | DAOSequence   |
 * //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * //       |                                                               |
 * //       +                                                               +
 * //       |                                                               |
 * //       +                            DODAGID*                           +
 * //       |                                                               |
 * //       +                                                               +
 * //       |                                                               |
 * //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * //       |   Option(s)...
 * //       +-+-+-+-+-+-+-+-+
 * </pre>
 */
class ICMPv6DIOMsg : public ::inet::ICMPv6Message
{
  protected:
    int code;
    int versionNumber;
    int rank;
    int grounded;
    int MOP;
    int DTSN;
    double IMin;
    int NofDoub;
    int k;
    IPv6Address DODAGID;
    int options;

  private:
    void copy(const ICMPv6DIOMsg& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const ICMPv6DIOMsg&);

  public:
    ICMPv6DIOMsg(const char *name=nullptr, short kind=0);
    ICMPv6DIOMsg(const ICMPv6DIOMsg& other);
    virtual ~ICMPv6DIOMsg();
    ICMPv6DIOMsg& operator=(const ICMPv6DIOMsg& other);
    virtual ICMPv6DIOMsg *dup() const override {return new ICMPv6DIOMsg(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int getCode() const;
    virtual void setCode(int code);
    virtual int getVersionNumber() const;
    virtual void setVersionNumber(int versionNumber);
    virtual int getRank() const;
    virtual void setRank(int rank);
    virtual int getGrounded() const;
    virtual void setGrounded(int grounded);
    virtual int getMOP() const;
    virtual void setMOP(int MOP);
    virtual int getDTSN() const;
    virtual void setDTSN(int DTSN);
    virtual double getIMin() const;
    virtual void setIMin(double IMin);
    virtual int getNofDoub() const;
    virtual void setNofDoub(int NofDoub);
    virtual int getK() const;
    virtual void setK(int k);
    virtual IPv6Address& getDODAGID();
    virtual const IPv6Address& getDODAGID() const {return const_cast<ICMPv6DIOMsg*>(this)->getDODAGID();}
    virtual void setDODAGID(const IPv6Address& DODAGID);
    virtual int getOptions() const;
    virtual void setOptions(int options);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const ICMPv6DIOMsg& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, ICMPv6DIOMsg& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>src/networklayer/icmpv6/ICMPv6MessageRPL.msg:205</tt> by nedtool.
 * <pre>
 * //EXTRA 
 * //The DAO control message for RPL, Section 6.4.1
 * //message ICMPv6DAOMsg extends ICMPv6Message
 * packet ICMPv6DAOMsg extends ICMPv6Message
 * {
 *     int code \@enum(ICMPv6_RPL_CONTROL_MSG); // RFC 6550, section 6: set to 0x01
 *     // TODO: checksum 
 *     //int rplInstanceId = 0; // RPLInstanceID: 8-bit field indicating the topology instance associated with the DODAG, as learned from the DIO.
 *     //int k;                // The 'K' flag indicates that the recipient is expected to send a DAO-ACK back.
 *     int dFlag;  // The 'D' flag indicates that the DODAGID field is present.  This flag MUST be set when a local RPLInstanceID is used.
 *     int kFlag;  //int flags = 0; // Flags: The 6 bits remaining unused in the Flags field are reserved for flags.  The field MUST be initialized to zero by the sender and MUST be ignored by the receiver.
 * 
 *     //int reserved = 0;  // Reserved: 8-bit unused field.  The field MUST be initialized to zero by the sender and MUST be ignored by the receiver.
 *     //int daoSequence;  //  DAOSequence: Incremented at each unique DAO message from a node and echoed in the DAO-ACK message.
 *     IPv6Address DODAGID;  //DODAGID (optional): 128-bit unsigned integer set by a DODAG root that uniquely identifies a DODAG.  This field is only present when the 'D' flag is set.  This field is typically only present when a local RPLInstanceID is in use, in order to identify the DODAGID that is associated with the RPLInstanceID.  When a global RPLInstanceID is in use, this field need not be present.
 * 
 *     int options \@enum(RPL_OPTIONS); //RFC 6550, section 6.3.1
 * 
 * 
 *     int prefixLen;        //When options are RPL_Target = 0x05;  
 *     IPv6Address prefix;   //When options are RPL_Target = 0x05;         
 *     simtime_t lifeTime;     //When option are Transit_Information = 0x06;
 *     IPv6Address daoParent;  //When option are Transit_Information; for Non-Storing
 * 
 * 
 * }
 * </pre>
 */
class ICMPv6DAOMsg : public ::inet::ICMPv6Message
{
  protected:
    int code;
    int dFlag;
    int kFlag;
    IPv6Address DODAGID;
    int options;
    int prefixLen;
    IPv6Address prefix;
    ::omnetpp::simtime_t lifeTime;
    IPv6Address daoParent;

  private:
    void copy(const ICMPv6DAOMsg& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const ICMPv6DAOMsg&);

  public:
    ICMPv6DAOMsg(const char *name=nullptr, short kind=0);
    ICMPv6DAOMsg(const ICMPv6DAOMsg& other);
    virtual ~ICMPv6DAOMsg();
    ICMPv6DAOMsg& operator=(const ICMPv6DAOMsg& other);
    virtual ICMPv6DAOMsg *dup() const override {return new ICMPv6DAOMsg(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int getCode() const;
    virtual void setCode(int code);
    virtual int getDFlag() const;
    virtual void setDFlag(int dFlag);
    virtual int getKFlag() const;
    virtual void setKFlag(int kFlag);
    virtual IPv6Address& getDODAGID();
    virtual const IPv6Address& getDODAGID() const {return const_cast<ICMPv6DAOMsg*>(this)->getDODAGID();}
    virtual void setDODAGID(const IPv6Address& DODAGID);
    virtual int getOptions() const;
    virtual void setOptions(int options);
    virtual int getPrefixLen() const;
    virtual void setPrefixLen(int prefixLen);
    virtual IPv6Address& getPrefix();
    virtual const IPv6Address& getPrefix() const {return const_cast<ICMPv6DAOMsg*>(this)->getPrefix();}
    virtual void setPrefix(const IPv6Address& prefix);
    virtual ::omnetpp::simtime_t getLifeTime() const;
    virtual void setLifeTime(::omnetpp::simtime_t lifeTime);
    virtual IPv6Address& getDaoParent();
    virtual const IPv6Address& getDaoParent() const {return const_cast<ICMPv6DAOMsg*>(this)->getDaoParent();}
    virtual void setDaoParent(const IPv6Address& daoParent);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const ICMPv6DAOMsg& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, ICMPv6DAOMsg& obj) {obj.parsimUnpack(b);}

} // namespace inet

#endif // ifndef __INET_ICMPV6MESSAGERPL_M_H
