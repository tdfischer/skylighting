#include <SoftwareSerial.h>
#include <LPD8806.h>

#include "graviton.h"
#include "animation.h"
#include "colors.h"

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
    uint32_t curColor = wheel (hue, strip());

    width += dir;
    if (width >= strip().numPixels()) {
      dir *= -1;
    } else if (width == 0) {
      pos += strip().numPixels() / 5;
      hue += 255/6;
      dir *= -1;
    }

    pos %= strip().numPixels();

    for (int i = 0; i < strip().numPixels(); i++) {
      if (i >= (int)(pos - width) && i <= (int)(pos + width))
        strip().setPixelColor (i, curColor);
    }

    strip().show();

    delay (10);
  }
};

bool paused = false;

GravitonVariant*
do_idle(int argc, const GravitonMethodArg* argv)
{
  paused = false;
  Serial.println("IDLE");
  return NULL;
}

GravitonVariant*
do_tweak(int argc, const GravitonMethodArg* argv)
{
  paused = !paused;
  return NULL;
}

GravitonMethod methods[] = {
  { "idle", do_idle },
  { "tweak", do_tweak }
};

Graviton graviton (&Serial, methods, 2);

SoftwareSerial ser = SoftwareSerial(2, 3);

LPD8806 strip(32);
DropAnimation idleAnim (&strip);
Animation* anim (&idleAnim);

void setup() {
  Serial.begin(9600);
  strip.begin();
  unsigned int i;
  for (i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor (i, 0);
  }
  strip.show();
}

void loop()
{
  graviton.loop();
  if (!paused)
    anim->step();
}

void serialEvent() {
  graviton.serialEvent ();
}
