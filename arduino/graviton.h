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
  GravitonReader(Stream* stream);
  void serialEvent();
  bool hasPacket();
  GravitonPacket& readPacket();

  enum ParserState {
    Empty,
    MethodCallLength,
    MethodCallName,
    Done
  };

private:
  Stream* m_stream;
  String m_buf;
  int m_curPacketBuf;
  int m_lastPacketBuf;
  GravitonPacket m_packets[PACKET_RINGBUF_SIZE];
  ParserState m_parserState;
  int m_tokenLength;
};

class Graviton {
public:
  Graviton(Stream* stream, const GravitonMethod* methods, int methodCount);

  void setProperty(const GravitonPropertySetPayload& payload);
  void getProperty(const GravitonPropertyGetPayload& payload);
  void callMethod(const GravitonMethodCallPayload& payload);
  void loop();
  void serialEvent();

private:
  String m_buf;
  GravitonReader m_reader;
  const GravitonMethod* m_methods;
  const int m_methodCount;
};

