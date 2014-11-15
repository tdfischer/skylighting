#include "graviton.h"

GravitonPacket::GravitonPacket() :
  valid (false)
{
}

GravitonReader::GravitonReader(Stream* stream) :
  m_stream (stream),
  m_curPacketBuf (0),
  m_lastPacketBuf (0),
  m_parserState (Empty)
{
}

void
GravitonReader::serialEvent()
{
  while (m_stream->available()) {
    char charIn = (char)m_stream->read();
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
        if (m_tokenLength == 0) {
          m_buf.toCharArray (cur.payload.methodCall.methodName, sizeof (cur.payload.methodCall.methodName));
          m_parserState = Done;
        }
        break;
    }

    if (m_parserState == Done)
      cur.valid = true;

    if (cur.valid)
      m_curPacketBuf++;
  }
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

Graviton::Graviton(Stream* stream, const GravitonMethod* methods, int methodCount) :
  m_reader (stream),
  m_methods (methods),
  m_methodCount (methodCount)
{
}

void
Graviton::callMethod(const GravitonMethodCallPayload& payload)
{
  int i;
  for (i = 0; i < m_methodCount; i++) {
    if (strcmp (m_methods[i].name, payload.methodName) == 0) {
      m_methods[i].func(0, 0);
    }
  }
}

void
Graviton::loop()
{
  if (m_reader.hasPacket()) {
    GravitonPacket pkt = m_reader.readPacket();
    switch (pkt.type) {
      /*case GravitonPacket::PropertySet:
        setProperty (pkt.payload.propertySet);
        break;
      case GravitonPacket::PropertyGet:
        getProperty (pkt.payload.propertyGet);
        break;*/
      case GravitonPacket::MethodCall:
        callMethod (pkt.payload.methodCall);
      default:
        break;
    }
  }
}

void
Graviton::serialEvent()
{
  m_reader.serialEvent ();
}
