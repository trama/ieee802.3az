//
// Generated file, do not edit! Created by opp_msgc 4.5 from src/EPLframe.msg.
//

#ifndef _EPLFRAME_M_H_
#define _EPLFRAME_M_H_

#include <omnetpp.h>

// opp_msgc version check
#define MSGC_VERSION 0x0405
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgc: 'make clean' should help.
#endif



/**
 * Enum generated from <tt>src/EPLframe.msg</tt> by opp_msgc.
 * <pre>
 * enum EPLframeMessageKind 
 * {
 *     PREQ = 0;
 *     PRES = 1;
 *     SOA = 2;
 *     SOC = 3;
 *     ASYNC = 4;
 *     STOP_IDLE = 5;
 *     LPI_SIGNAL = 6;
 *     EXIT_PLPI = 7;
 *     RETURN_PLPI = 8;
 * };
 * </pre>
 */
enum EPLframeMessageKind {
    PREQ = 0,
    PRES = 1,
    SOA = 2,
    SOC = 3,
    ASYNC = 4,
    STOP_IDLE = 5,
    LPI_SIGNAL = 6,
    EXIT_PLPI = 7,
    RETURN_PLPI = 8
};

/**
 * Class generated from <tt>src/EPLframe.msg</tt> by opp_msgc.
 * <pre>
 * packet EPLframe {
 *     int type @enum(EPLframeMessageKind); 
 *     int host; 
 *     int mp; 
 *     bool partialLPI;
 * }
 * </pre>
 */
class EPLframe : public ::cPacket
{
  protected:
    int type_var;
    int host_var;
    int mp_var;
    bool partialLPI_var;

  private:
    void copy(const EPLframe& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const EPLframe&);

  public:
    EPLframe(const char *name=NULL, int kind=0);
    EPLframe(const EPLframe& other);
    virtual ~EPLframe();
    EPLframe& operator=(const EPLframe& other);
    virtual EPLframe *dup() const {return new EPLframe(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual int getType() const;
    virtual void setType(int type);
    virtual int getHost() const;
    virtual void setHost(int host);
    virtual int getMp() const;
    virtual void setMp(int mp);
    virtual bool getPartialLPI() const;
    virtual void setPartialLPI(bool partialLPI);
};

inline void doPacking(cCommBuffer *b, EPLframe& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, EPLframe& obj) {obj.parsimUnpack(b);}


#endif // _EPLFRAME_M_H_
