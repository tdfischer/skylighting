#include "graviton.h"

GravitonPacket::GravitonPacket() :
  valid (false)
{
}

GravitonReader::GravitonReader() :
  m_buf (""),
  m_bufHead (0),
  m_bufTail (0),
  m_curPacketBuf (0),
  m_lastPacketBuf (0),
  m_parserState (Empty)
{
  m_buf.reserve (sizeof (m_streamBuf));
}

void
GravitonReader::handleBuffer()
{
  while (m_bufHead != m_bufTail) {
    unsigned char charIn = m_streamBuf[m_bufHead];
    GravitonPacket& cur = m_packets[m_curPacketBuf];
    switch (m_parserState) {
      case Empty:
        cur.type = (GravitonPacket::Type)charIn;
        m_buf = "";
        switch (cur.type) {
          case GravitonPacket::MethodCall:
            m_parserState = MethodCallLength;
            break;
          default:
            m_parserState = Empty;
        }
        break;
      case MethodCallLength:
        m_tokenLength = charIn;
        m_parserState = MethodCallName;
        m_buf = "";
        break;
      case MethodCallName:
        m_tokenLength--;
        m_buf += charIn;
        if (m_tokenLength <= 0) {
          m_buf.toCharArray (cur.payload.methodCall.methodName, sizeof (cur.payload.methodCall.methodName));
          m_parserState = Done;
        }
        break;
    }
    if (m_parserState == Done) {
      cur.valid = true;
      m_curPacketBuf++;
      m_parserState = Empty;
    }

    m_bufHead++;
    m_bufHead %= 128;
  }
}

void
GravitonReader::appendBuffer(unsigned char c)
{
  m_streamBuf[m_bufTail] = c;
  m_bufTail++;
  m_bufTail %= 128;
}

bool
GravitonReader::hasPacket()
{
  return m_packets[m_lastPacketBuf].valid;
}

GravitonPacket&
GravitonReader::readPacket()
{
  if (m_lastPacketBuf >= PACKET_RINGBUF_SIZE)
    m_lastPacketBuf = 0;
  return m_packets[m_lastPacketBuf++];
}

Graviton::Graviton(GravitonReader* reader, const GravitonService* services, int serviceCount) :
  m_reader (reader),
  m_services (services),
  m_serviceCount (serviceCount)
{
}

GravitonVariant*
Graviton::callMethod(const GravitonMethodCallPayload& payload)
{
  int i;
  int j;
  for (i = 0; i < m_serviceCount; i++) {
    if (strcmp (m_services[i].name, payload.serviceName) == 0) {
      const GravitonService& svc = m_services[i];
      for (j = 0; j < svc.methodCount; j++) {
        if (strcmp (svc.methods[i].name, payload.methodName) == 0) {
          return svc.methods[i].func(0, 0);
        }
      }
    }
  }
  return nullptr;
}

void
Graviton::loop()
{
  m_reader->handleBuffer();
  if (m_reader->hasPacket()) {
    GravitonPacket pkt = m_reader->readPacket();
    switch (pkt.type) {
      /*case GravitonPacket::PropertySet:
        setProperty (pkt.payload.propertySet);
        break;
      case GravitonPacket::PropertyGet:
        getProperty (pkt.payload.propertyGet);
        break;*/
      case GravitonPacket::MethodCall:
        m_reader->reply (pkt, callMethod (pkt.payload.methodCall));
      default:
        break;
    }
  }
}


GravitonVariant*
do_ping(int argc, const GravitonMethodArg* arg)
{
  static GravitonVariant ret ( "pong" );
  return &ret;
}

static const GravitonMethod introspectionMethods[] = {
  { "ping", do_ping},
};

static const GravitonService introspectionService = {
  "net.phrobo.graviton.introspection",
  1,
  introspectionMethods
};

const GravitonService* GravitonIntrospectionService (&introspectionService);
