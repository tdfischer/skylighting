#include "graviton.h"

GravitonPacket::GravitonPacket() :
  valid (false)
{
}

GravitonReader::GravitonReader() :
  m_bufHead (0),
  m_bufTail (0),
  m_curPacketBuf (0),
  m_parserBufIdx (0),
  m_lastPacketBuf (0),
  m_parserState (Empty)
{
}

GravitonReader::ParserState
GravitonReader::parserState() const
{
  return m_parserState;
}

void
GravitonReader::parseString(char* buf, unsigned char c, unsigned int maxSize)
{
  buf[m_parserBufIdx++] = c;
  if (c == 0 || m_parserBufIdx >= maxSize) {
    m_parserState = GravitonReader::Done;
    m_parserBufIdx = 0;
  }
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
        switch (cur.type) {
          case GravitonPacket::FindService:
            m_parserState = FindServiceName;
            break;
          case GravitonPacket::FindMethod:
            m_parserState = FindMethodServiceIdx;
            break;
          case GravitonPacket::MethodCall:
            m_parserState = MethodCallServiceIdx;
            break;
          default:
            m_parserState = Empty;
        }
        break;
      case FindServiceName:
        parseString ((char*)&cur.payload.findService.name,
                     charIn,
                     sizeof (cur.payload.findService.name));
        break;
      case FindMethodServiceIdx:
        cur.payload.findMethod.serviceIdx = charIn;
        m_parserState = FindMethodName;
        break;
      case FindMethodName:
        parseString ((char*)&cur.payload.findMethod.name,
                     charIn,
                     sizeof (cur.payload.findMethod.name));
        break;

      case MethodCallServiceIdx:
        cur.payload.methodCall.serviceIdx = charIn;
        m_parserState = MethodCallMethodIdx;
        break;
      case MethodCallMethodIdx:
        cur.payload.methodCall.methodIdx = charIn;
        m_parserState = Done;
        break;
    }

    if (m_parserState == Done) {
      cur.valid = true;
      m_curPacketBuf++;
      m_curPacketBuf %= PACKET_RINGBUF_SIZE;
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
  GravitonPacket& ret = m_packets[m_lastPacketBuf++];
  ret.valid = false;
  m_lastPacketBuf %= PACKET_RINGBUF_SIZE;
  return ret;
}

Graviton::Graviton(GravitonReader* reader, const GravitonService* services, unsigned char serviceCount) :
  m_reader (reader),
  m_services (services),
  m_serviceCount (serviceCount)
{
}

GravitonMethod&
GravitonService::methodByIdx(unsigned char idx) const
{
  return const_cast<GravitonMethod&>(methods[idx]);
}

GravitonService&
Graviton::serviceByIdx(unsigned char idx) const
{
  return const_cast<GravitonService&>(m_services[idx]);
}

void
Graviton::callMethod(const GravitonMethodCallPayload& payload, GravitonVariant* ret)
{
  if (payload.serviceIdx < m_serviceCount) {
    const GravitonService& svc = m_services[payload.serviceIdx];
    if (payload.methodIdx < svc.methodCount) {
      return svc.methods[payload.methodIdx].func(0, 0, ret);
    }
  }
}

void
Graviton::findService(const GravitonFindServicePayload& payload, GravitonVariant* ret) const
{
  for (unsigned char i = 0; i < m_serviceCount;i++) {
    if (strcmp (payload.name, m_services[i].name) == 0) {
      *ret = GravitonVariant (i);
      return;
    }
  }

  *ret = GravitonVariant (-1);
}

void
Graviton::findMethod(const GravitonFindMethodPayload& payload, GravitonVariant* ret) const
{
  const GravitonService& svc = m_services[payload.serviceIdx];
  for (unsigned char i = 0; i < svc.methodCount; i++) {
    if (strcmp (payload.name, svc.methods[i].name) == 0) {
      *ret = GravitonVariant (i);
      return;
    }
  }
  *ret = GravitonVariant (-1);
}

void
Graviton::loop()
{
  m_reader->handleBuffer();
  if (m_reader->hasPacket()) {
    GravitonPacket& pkt = m_reader->readPacket();
    GravitonVariant reply;
    switch (pkt.type) {
      /*case GravitonPacket::PropertySet:
        setProperty (pkt.payload.propertySet);
        break;
      case GravitonPacket::PropertyGet:
        getProperty (pkt.payload.propertyGet);
        break;*/
      case GravitonPacket::FindService:
        findService (pkt.payload.findService, &reply);
        m_reader->reply (pkt, &reply);
        break;
      case GravitonPacket::FindMethod:
        findMethod (pkt.payload.findMethod, &reply);
        m_reader->reply (pkt, &reply);
        break;
      case GravitonPacket::MethodCall:
        callMethod (pkt.payload.methodCall, &reply);
        m_reader->reply (pkt, &reply);
      default:
        break;
    }
  }
}

void
do_peerCount(unsigned char argc, const GravitonMethodArg* arg, GravitonVariant* ret)
{
  *ret = GravitonVariant (0);
}

static const GravitonMethod introspectionMethods[] = {
  { "peerCount", do_peerCount},
};

const GravitonService Graviton::introspectionService = {
  "net.phrobo.graviton.introspection",
  1,
  introspectionMethods
};

void
GravitonReader::resetParser()
{
  m_parserState = Empty;
}
