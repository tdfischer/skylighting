#include <SoftwareSerial.h>
#include <LPD8806.h>
#include <XBee.h>
#include <Graviton.h>
#include <graviton-xbee-reader.h>

#include "animation.h"
#include "colors.h"

#ifdef DEBUG
SoftwareSerial dbg_ser (2, 3);
#endif // DEBUG

LPD8806 strip(32);

class HouselightsAnimation : public Animation {
public:
  HouselightsAnimation() :
    Animation()
  {}

  void redraw()
  {
    for(uint16_t  i = 0; i < strip.numPixels();i++) {
      strip.setPixelColor (i, random (230, 255), random (230, 255), random (230, 255));
    }
    strip.show();
    delay(1000);
  }
};

class DirectionAnimation : public Animation {
public:
  DirectionAnimation() :
    Animation()
  {}

  uint8_t hue;

  void redraw()
  {
    uint8_t pos = idx() % strip.numPixels();
    hue++;
    for (uint16_t  i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor (i, wheel (hue, 255 * ((pos-i)/(double)strip.numPixels()), strip));
    }
    strip.show();
    delay(30);
  }
};

class DropAnimation : public Animation {
public:
  DropAnimation() :
    Animation(),
    pos(strip.numPixels()/2),
    dir(1),
    width(0)
  {}

  uint16_t pos;
  uint8_t dir;
  uint16_t width;
  uint8_t hue;

  void redraw()
  {
    uint32_t curColor = wheel (hue, 255, strip);

    width += dir;
    if (width >= strip.numPixels()) {
      dir *= -1;
    } else if (width == 0) {
      for (uint8_t b = 255; b > 0; b--) {
        curColor = wheel (hue, b, strip);
        for(uint16_t i = 0; i < strip.numPixels(); i++) {
          strip.setPixelColor (i, curColor);
        }
        strip.show();
        delay(30);
      }

      pos += strip.numPixels() / 5;
      hue += 255/6;
      dir *= -1;

      for (uint8_t b = 128; b > 0; b-=2) {
        strip.setPixelColor (pos, b, b, b);
        strip.show();
        delay(5);
      }
    }

    pos %= strip.numPixels();

    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      if (i >= (pos - width) && i <= (pos + width))
        strip.setPixelColor (i, curColor);
    }

    strip.show();

    delay (10);
  }
};

DropAnimation idleAnim;
HouselightsAnimation houselightsAnim;
DirectionAnimation directionAnim;
Animation* anim (&idleAnim);

void
do_idle(unsigned char argc, const GravitonMethodArg* argv, GravitonVariant* ret)
{
  anim = &idleAnim;
#ifdef DEBUG
  dbg_ser.println (F ("Idle!"));
#endif // DEBUG
}

void
do_directional(unsigned char argc, const GravitonMethodArg* argv, GravitonVariant* ret)
{
  anim = &directionAnim;
#ifdef DEBUG
  dbg_ser.println (F ("Directional!"));
#endif // DEBUG
}

void
do_houselights(unsigned char argc, const GravitonMethodArg* argv, GravitonVariant* ret)
{
  anim = &houselightsAnim;
#ifdef DEBUG
  dbg_ser.println (F ("House Lights!"));
#endif // DEBUG
}

const GravitonMethod methods[] = {
  { "idle", do_idle },
  { "directional", do_directional },
  { "houselights", do_houselights }
};

const GravitonService services[2] = {
  GravitonService( 
    "net.phrobo.brilliance.linear",
    3,
    methods
  ),
  GravitonIntrospectionService()
};

XBee bee;
GravitonXBeeReader reader (&bee);

GRAVITON_STATIC_INIT (&reader, services, 2);

void setup() {
#ifdef DEBUG
  dbg_ser.begin (9600);
  dbg_ser.println ( F ("Boot"));
#endif // DEBUG

  Serial.begin (9600);
  strip.begin();
  randomSeed (analogRead(0));
  idleAnim.hue = random (255);
  directionAnim.hue = random (255);
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
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
  Graviton.loop();
  anim->step();
}

void serialEvent() {
  reader.serialEvent ();
}
