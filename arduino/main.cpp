#include <SoftwareSerial.h>
#include <LPD8806.h>
#include <XBee.h>

#include "graviton.h"
#include "animation.h"
#include "colors.h"

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

GravitonVariant*
do_idle(int argc, const GravitonMethodArg* argv)
{
  anim = &idleAnim;
  return NULL;
}

GravitonVariant*
do_directional(int argc, const GravitonMethodArg* argv)
{
  animLoops = 15;
  anim = &directionAnim;
  return NULL;
}

GravitonVariant*
do_houselights(int argc, const GravitonMethodArg* argv)
{
  anim = &houselightsAnim;
  return NULL;
}

GravitonMethod methods[] = {
  { "idle", do_idle },
  { "directional", do_directional },
  { "houselights", do_houselights }
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

private:
  XBee* m_xbee;
};

XBee bee;

GravitonXBeeReader reader (&bee);

Graviton graviton (&reader, methods, 3);

SoftwareSerial ser (2, 3);

void setup() {
  ser.begin (9600);
  bee.setSerial (ser);
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
