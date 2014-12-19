#include <SoftwareSerial.h>
#include <LPD8806.h>
#include <XBee.h>

#include "graviton.h"
#include "animation.h"
#include "colors.h"

SoftwareSerial dbg_ser (2, 3);

class HouselightsAnimation : public Animation {
public:
  HouselightsAnimation(LPD8806* strip) :
    Animation(strip, 10)
  {}

  void redraw()
  {
    for(unsigned int i = 0; i < strip().numPixels();i++) {
      strip().setPixelColor (i, random (230, 255), random (230, 255), random (230, 255));
    }
    strip().show();
    delay(1000);
    end();
  }
};

class DirectionAnimation : public Animation {
public:
  DirectionAnimation(LPD8806* strip) :
    Animation(strip)
  {}

  uint8_t hue;

  void redraw()
  {
    int pos = idx() % strip().numPixels();
    hue++;
    for (unsigned int i = 0; i < strip().numPixels(); i++) {
      strip().setPixelColor (i, wheel (hue, 255 * ((pos-i)/(double)strip().numPixels()), strip()));
    }
    strip().show();
    delay(30);
  }
};

class DropAnimation : public Animation {
public:
  DropAnimation(LPD8806* strip) :
    Animation(strip),
    pos(strip->numPixels()/2),
    dir(1),
    width(0)
  {}

  unsigned int pos;
  int dir;
  unsigned int width;
  uint8_t hue;

  void redraw()
  {
    uint32_t curColor = wheel (hue, 255, strip());

    width += dir;
    if (width >= strip().numPixels()) {
      dir *= -1;
    } else if (width == 0) {
      for (unsigned int b = 255; b > 0; b--) {
        curColor = wheel (hue, b, strip());
        for(unsigned int i = 0; i < strip().numPixels(); i++) {
          strip().setPixelColor (i, curColor);
        }
        strip().show();
        delay(30);
      }

      pos += strip().numPixels() / 5;
      hue += 255/6;
      dir *= -1;

      for (int b = 128; b > 0; b-=2) {
        strip().setPixelColor (pos, b, b, b);
        strip().show();
        delay(5);
      }
    }

    pos %= strip().numPixels();

    for (unsigned int i = 0; i < strip().numPixels(); i++) {
      if (i >= (unsigned int)(pos - width) && i <= (unsigned int)(pos + width))
        strip().setPixelColor (i, curColor);
    }

    strip().show();

    delay (10);
  }
};

int animLoops = 500;

LPD8806 strip(32);
DropAnimation idleAnim (&strip);
HouselightsAnimation houselightsAnim (&strip);
DirectionAnimation directionAnim (&strip);
Animation* anim (&houselightsAnim);

void
do_idle(unsigned char argc, const GravitonMethodArg* argv, GravitonVariant* ret)
{
  anim = &idleAnim;
  dbg_ser.println ("Idle!");
}

void
do_directional(unsigned char argc, const GravitonMethodArg* argv, GravitonVariant* ret)
{
  animLoops = 15;
  anim = &directionAnim;
  dbg_ser.println ("Directional!");
}

void
do_houselights(unsigned char argc, const GravitonMethodArg* argv, GravitonVariant* ret)
{
  anim = &houselightsAnim;
  dbg_ser.println ("House Lights!");
}

GravitonMethod methods[] = {
  { "idle", do_idle },
  { "directional", do_directional },
  { "houselights", do_houselights }
};

GravitonService services[2] = {
  {
    "net.phrobo.brilliance.linear",
    3,
    methods
  },
  Graviton::introspectionService
};

class GravitonStreamReader : public GravitonReader {
public:
  GravitonStreamReader (Stream& s) :
    m_stream (s)
  {}

  void serialEvent()
  {
    while (m_stream.available()) {
      char c = m_stream.read();
      if (c == -1) {
        resetParser();
      } else {
        appendBuffer (c);
      }
    }
  }

  void reply (const GravitonPacket& pkt, GravitonVariant* ret)
  {
    uint8_t b = ret->type;
    m_stream.write ((uint8_t*)&b, sizeof (b));
    switch (ret->type) {
      case GravitonVariant::Null:
        break;
      case GravitonVariant::Integer:
        m_stream.write ((uint8_t*)&ret->value.asInt, sizeof (ret->value.asInt));
        break;
      case GravitonVariant::String:
        m_stream.write ((uint8_t*)ret->value.asString, strlen (ret->value.asString)+1);
        break;
    }
  }

private:
  Stream& m_stream;
};

class GravitonXBeeReader : public GravitonReader {
public:
  GravitonXBeeReader (XBee* xbee) :
    m_xbee (xbee)
  {}

  void serialEvent()
  {
    m_xbee->readPacket();
    if (m_xbee->getResponse().isAvailable()) {
      if (m_xbee->getResponse().getApiId() == RX_64_RESPONSE) {
        Rx64Response rx;
        uint8_t* pktBuf;
        m_xbee->getResponse().getRx64Response (rx);
        pktBuf = rx.getData();
        for (int i = 0; i < rx.getDataLength(); i++) {
          appendBuffer (pktBuf[i]);
        }
      }
    }
  }
  
  void reply (const GravitonPacket& pkt, GravitonVariant* ret)
  {
  }

private:
  XBee* m_xbee;
};

XBee bee;
GravitonXBeeReader reader (&bee);

//GravitonStreamReader reader (Serial);

Graviton graviton (&reader, services, 2);

void setup() {
  dbg_ser.begin (9600);
  dbg_ser.println ( F ("Boot"));

  Serial.begin (9600);
  //bee.setSerial (Serial);
  strip.begin();
  randomSeed (analogRead(0));
  idleAnim.hue = random (255);
  directionAnim.hue = random (255);
  unsigned int i;
  for (i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor (i, 0);
  }
  strip.show();
}

int loopNum;

void loop()
{
  if (anim->isFinished()) {
    anim->reset();
    anim = &idleAnim;
  }
  graviton.loop();
  anim->step();
}

void serialEvent() {
  reader.serialEvent ();
}
