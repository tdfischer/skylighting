#include <WString.h>
#include <Stream.h>

struct GravitonVariant {
  enum Type {
    Integer,
    String
  };

  Type type;

  union {
    int asInt;
    char asString[16];
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

struct GravitonMethodCallPayload {
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
  Graviton(GravitonReader* reader, const GravitonMethod* methods, int methodCount);

  void setProperty(const GravitonPropertySetPayload& payload);
  void getProperty(const GravitonPropertyGetPayload& payload);
  void callMethod(const GravitonMethodCallPayload& payload);
  void loop();

private:
  String m_buf;
  GravitonReader* m_reader;
  const GravitonMethod* m_methods;
  const int m_methodCount;
};

