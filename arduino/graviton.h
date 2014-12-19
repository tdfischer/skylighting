#include <WString.h>
#include <Stream.h>

struct GravitonVariant {
  GravitonVariant() :
    type (Null),
    value (nullptr) {}

  explicit GravitonVariant(const char* str) :
    type (String),
    value (str) {}

  explicit GravitonVariant(int i) :
    type(Integer),
    value(i) {}

  enum Type {
    Null,
    Integer,
    String
  };

  Type type;

  union v {
    int asInt;
    const char *asString;
    v(const char* str) : asString (str) {}
    v(int i) : asInt (i) {}
  } value;
};

struct GravitonMethodArg {
  char name[16];
  union {
    int asInt;
    char asString[16];
  } value;
};

typedef void (*GravitonMethodFunc)(unsigned char argc, const GravitonMethodArg* argv, GravitonVariant* ret);

#ifndef GRAVITON_METHOD_NAME_MAXLEN
#define GRAVITON_METHOD_NAME_MAXLEN 16
#endif // GRAVITON_METHOD_NAME_MAXLEN

struct GravitonMethod {
  const char name[GRAVITON_METHOD_NAME_MAXLEN];
  const GravitonMethodFunc func;
};

#ifndef GRAVITON_SERVICE_NAME_MAXLEN
#define GRAVITON_SERVICE_NAME_MAXLEN 35
#endif // GRAVITON_SERVICE_NAME_MAXLEN

struct GravitonService {
  const char name[GRAVITON_SERVICE_NAME_MAXLEN];
  unsigned char methodCount;
  const GravitonMethod* methods;

  GravitonMethod& methodByIdx(unsigned char idx) const;
};

struct GravitonMethodCallPayload {
  unsigned char serviceIdx;
  unsigned char methodIdx;
  GravitonMethodArg args[5];
};

struct GravitonPropertySetPayload {
};

struct GravitonPropertyGetPayload {
};

struct GravitonFindServicePayload {
  char name[GRAVITON_SERVICE_NAME_MAXLEN+1];
};

struct GravitonFindMethodPayload {
  unsigned char serviceIdx;
  char name[GRAVITON_METHOD_NAME_MAXLEN];
};

struct GravitonPacket {
  GravitonPacket();

  bool valid;
  enum Type {
    FindService,
    FindMethod,
    MethodCall,
    PropertyGet,
    PropertySet
  };

  Type type;

  union {
    GravitonFindServicePayload findService;
    GravitonFindMethodPayload findMethod;
    GravitonPropertySetPayload propertySet;
    GravitonPropertyGetPayload propertyGet;
    GravitonMethodCallPayload methodCall;
  } payload;
};

#define PACKET_RINGBUF_SIZE 5

class GravitonReader {
public:
  enum ParserState {
    Empty,
    FindServiceName,
    FindMethodServiceIdx,
    FindMethodName,
    MethodCallServiceIdx,
    MethodCallMethodIdx,
    Done
  };

  GravitonReader();
  void handleBuffer();
  bool hasPacket();
  GravitonPacket& readPacket();
  virtual void reply (const GravitonPacket& pkt, GravitonVariant* ret) = 0;
  ParserState parserState() const;
  void resetParser();

protected:
  void appendBuffer(unsigned char c);

private:
  void parseString(char* buf, unsigned char c, unsigned int maxSize);

  unsigned char m_streamBuf[128];
  int m_bufHead;
  int m_bufTail;
  int m_curPacketBuf;
  unsigned int m_parserBufIdx;
  int m_lastPacketBuf;
  GravitonPacket m_packets[PACKET_RINGBUF_SIZE];
  ParserState m_parserState;
};

class Graviton {
public:
  Graviton(GravitonReader* reader, const GravitonService* services, unsigned char serviceCount);

  void setProperty(const GravitonPropertySetPayload& payload);
  void getProperty(const GravitonPropertyGetPayload& payload) const;
  void callMethod(const GravitonMethodCallPayload& payload, GravitonVariant* ret);
  void findService(const GravitonFindServicePayload& payload, GravitonVariant* ret) const;
  void findMethod(const GravitonFindMethodPayload& payload, GravitonVariant* ret) const;
  void loop();

  GravitonService& serviceByIdx(unsigned char idx) const;

  static const GravitonService introspectionService;

private:
  String m_buf;
  GravitonReader* m_reader;
  const GravitonService* m_services;
  const unsigned char m_serviceCount;
};
