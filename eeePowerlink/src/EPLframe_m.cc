//
// Generated file, do not edit! Created by opp_msgc 4.5 from src/EPLframe.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "EPLframe_m.h"

USING_NAMESPACE


// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}




// Template rule for outputting std::vector<T> types
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');
    
    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("EPLframeMessageKind");
    if (!e) enums.getInstance()->add(e = new cEnum("EPLframeMessageKind"));
    e->insert(PREQ, "PREQ");
    e->insert(PRES, "PRES");
    e->insert(SOA, "SOA");
    e->insert(SOC, "SOC");
    e->insert(ASYNC, "ASYNC");
    e->insert(STOP_IDLE, "STOP_IDLE");
    e->insert(LPI_SIGNAL, "LPI_SIGNAL");
    e->insert(EXIT_PLPI, "EXIT_PLPI");
    e->insert(RETURN_PLPI, "RETURN_PLPI");
);

Register_Class(EPLframe);

EPLframe::EPLframe(const char *name, int kind) : ::cPacket(name,kind)
{
    this->type_var = 0;
    this->host_var = 0;
    this->mp_var = 0;
    this->partialLPI_var = 0;
}

EPLframe::EPLframe(const EPLframe& other) : ::cPacket(other)
{
    copy(other);
}

EPLframe::~EPLframe()
{
}

EPLframe& EPLframe::operator=(const EPLframe& other)
{
    if (this==&other) return *this;
    ::cPacket::operator=(other);
    copy(other);
    return *this;
}

void EPLframe::copy(const EPLframe& other)
{
    this->type_var = other.type_var;
    this->host_var = other.host_var;
    this->mp_var = other.mp_var;
    this->partialLPI_var = other.partialLPI_var;
}

void EPLframe::parsimPack(cCommBuffer *b)
{
    ::cPacket::parsimPack(b);
    doPacking(b,this->type_var);
    doPacking(b,this->host_var);
    doPacking(b,this->mp_var);
    doPacking(b,this->partialLPI_var);
}

void EPLframe::parsimUnpack(cCommBuffer *b)
{
    ::cPacket::parsimUnpack(b);
    doUnpacking(b,this->type_var);
    doUnpacking(b,this->host_var);
    doUnpacking(b,this->mp_var);
    doUnpacking(b,this->partialLPI_var);
}

int EPLframe::getType() const
{
    return type_var;
}

void EPLframe::setType(int type)
{
    this->type_var = type;
}

int EPLframe::getHost() const
{
    return host_var;
}

void EPLframe::setHost(int host)
{
    this->host_var = host;
}

int EPLframe::getMp() const
{
    return mp_var;
}

void EPLframe::setMp(int mp)
{
    this->mp_var = mp;
}

bool EPLframe::getPartialLPI() const
{
    return partialLPI_var;
}

void EPLframe::setPartialLPI(bool partialLPI)
{
    this->partialLPI_var = partialLPI;
}

class EPLframeDescriptor : public cClassDescriptor
{
  public:
    EPLframeDescriptor();
    virtual ~EPLframeDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(EPLframeDescriptor);

EPLframeDescriptor::EPLframeDescriptor() : cClassDescriptor("EPLframe", "cPacket")
{
}

EPLframeDescriptor::~EPLframeDescriptor()
{
}

bool EPLframeDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<EPLframe *>(obj)!=NULL;
}

const char *EPLframeDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int EPLframeDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 4+basedesc->getFieldCount(object) : 4;
}

unsigned int EPLframeDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<4) ? fieldTypeFlags[field] : 0;
}

const char *EPLframeDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "type",
        "host",
        "mp",
        "partialLPI",
    };
    return (field>=0 && field<4) ? fieldNames[field] : NULL;
}

int EPLframeDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='t' && strcmp(fieldName, "type")==0) return base+0;
    if (fieldName[0]=='h' && strcmp(fieldName, "host")==0) return base+1;
    if (fieldName[0]=='m' && strcmp(fieldName, "mp")==0) return base+2;
    if (fieldName[0]=='p' && strcmp(fieldName, "partialLPI")==0) return base+3;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *EPLframeDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "int",
        "int",
        "bool",
    };
    return (field>=0 && field<4) ? fieldTypeStrings[field] : NULL;
}

const char *EPLframeDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0:
            if (!strcmp(propertyname,"enum")) return "EPLframeMessageKind";
            return NULL;
        default: return NULL;
    }
}

int EPLframeDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    EPLframe *pp = (EPLframe *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string EPLframeDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    EPLframe *pp = (EPLframe *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getType());
        case 1: return long2string(pp->getHost());
        case 2: return long2string(pp->getMp());
        case 3: return bool2string(pp->getPartialLPI());
        default: return "";
    }
}

bool EPLframeDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    EPLframe *pp = (EPLframe *)object; (void)pp;
    switch (field) {
        case 0: pp->setType(string2long(value)); return true;
        case 1: pp->setHost(string2long(value)); return true;
        case 2: pp->setMp(string2long(value)); return true;
        case 3: pp->setPartialLPI(string2bool(value)); return true;
        default: return false;
    }
}

const char *EPLframeDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    };
}

void *EPLframeDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    EPLframe *pp = (EPLframe *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


