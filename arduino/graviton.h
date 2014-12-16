#include <WString.h>
#include <Stream.h>

struct GravitonVariant {
  GravitonVariant(const char* str) :
    type (Integer),
    value (str) {}

  enum Type {
    Integer,
    String
  };

  Type type;

  union v {
    int asInt;
    const char *asString;
    v(const char* str) : asString (str) {}
  } value;
};

struct GravitonMethodArg {
  char name[16];
  union {
    int asInt;
    char asString[16];
  } value;
};

typedef GravitonVariant* (*GravitonMethodFunc)(int argc, const GravitonMethodArg* argv);

struct GravitonMethod {
  const char name[16];
  const GravitonMethodFunc func;
};

struct GravitonService {
  const char name[48];
  int methodCount;
  const GravitonMethod* methods;
};

struct GravitonMethodCallPayload {
  char serviceName[48];
  char methodName[16];
  GravitonMethodArg args[5];
};

struct GravitonPropertySetPayload {
};

struct GravitonPropertyGetPayload {
};

struct GravitonPacket {
  GravitonPacket();

  bool valid;
  enum Type {
    MethodCall,
    PropertyGet,
    PropertySet
  };

  Type type;

  union {
    GravitonPropertySetPayload propertySet;
    GravitonPropertyGetPayload propertyGet;
    GravitonMethodCallPayload methodCall;
  } payload;
};

#define PACKET_RINGBUF_SIZE 5

class GravitonReader {
public:
  GravitonReader();
  void handleBuffer();
  bool hasPacket();
  GravitonPacket& readPacket();
  virtual void reply (const GravitonPacket& pkt, GravitonVariant* ret) = 0;

  enum ParserState {
    Empty,
    MethodCallLength,
    MethodCallName,
    Done
  };

protected:
  void appendBuffer(unsigned char c);

private:
  String m_buf;
  unsigned char m_streamBuf[128];
  int m_bufHead;
  int m_bufTail;
  int m_curPacketBuf;
  int m_lastPacketBuf;
  GravitonPacket m_packets[PACKET_RINGBUF_SIZE];
  ParserState m_parserState;
  int m_tokenLength;
};

class Graviton {
public:
  Graviton(GravitonReader* reader, const GravitonService* services, int serviceCount);

  void setProperty(const GravitonPropertySetPayload& payload);
  void getProperty(const GravitonPropertyGetPayload& payload);
  GravitonVariant* callMethod(const GravitonMethodCallPayload& payload);
  void loop();

private:
  String m_buf;
  GravitonReader* m_reader;
  const GravitonService* m_services;
  const int m_serviceCount;
};

