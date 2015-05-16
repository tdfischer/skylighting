#include <SoftwareSerial.h>
#include <FastLED.h>
#include <XBee.h>
#include <Graviton.h>
#include <graviton-xbee-reader.h>

#include "animation.h"

#ifdef DEBUG
SoftwareSerial dbg_ser (2, 3);
#endif // DEBUG

#define LED_COUNT 346
#define LED_COUNT 93
CRGB leds[LED_COUNT];

class HouselightsAnimation : public Animation {
public:
  HouselightsAnimation() :
    Animation()
  {}

  void redraw()
  {
    for(uint16_t  i = 0; i < LED_COUNT;i++) {
      leds[i] = CHSV (random(230, 255), 10, 255);
    }
    FastLED.show();
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
    uint8_t pos = idx() % LED_COUNT;
    hue++;
    for (uint16_t  i = 0; i < LED_COUNT; i++) {
      leds[i] = CHSV(hue, 255, 255 * ((pos - i)/(double)LED_COUNT));
    }
    FastLED.show();
    delay(30);
  }
};

class DropAnimation : public Animation {
public:
  DropAnimation() :
    Animation(),
    pos(LED_COUNT/2),
    width(0),
    max_brightness(255),
    render_func(&DropAnimation::do_starburst)
  {}

  uint16_t pos;
  uint16_t width;
  uint16_t hue;
  uint8_t max_brightness;
  uint8_t brightness;

  /**
   * render_func: Current sub-animation to render
   */
  void (DropAnimation::*render_func)();

  /**
   * redraw: Redraw the current frame
   */
  void redraw()
  {
    (this->*render_func)();
  }

  /**
   * do_starburst: Initial point of light starburst animation thingee
   */
  void do_starburst() {
    for (uint8_t b = 128; b > 0; b-=2) {
      leds[pos] = CHSV(255, 0, b);
      FastLED.show();
      delay(5);
    }

    render_func = &DropAnimation::do_expand;
    width = 0;
  }

  /**
   * do_expand: Expansion animation
   */
  void do_expand() {
    width++;
    hue++;
    hue %= 128 * 3;
    CHSV curColor = CHSV (hue, 255, max_brightness);

    uint16_t start, end;
    start = max(pos - width, 0);
    end = min(LED_COUNT, pos + width);

    for (uint16_t i = start; i < end; i++) {
      leds[i] = curColor;
    }

    FastLED.show();
    delay(10);

    if (width >= LED_COUNT) {
      render_func = &DropAnimation::do_fadeout;
    }
  }

  /**
   * do_fadeout: Fade out animation
   */
  void do_fadeout() {
    brightness--;
    CHSV curColor = CHSV (hue, 255, min(max_brightness, brightness));
    for (uint16_t i = 0; i < LED_COUNT;i++) {
      leds[i] = curColor;
    }

    FastLED.show();

    if (brightness == 0) {
      render_func = &DropAnimation::do_starburst;
      pos++;
      pos %= LED_COUNT;
    }
    delay (10);
    hue++;
    hue %= 128 * 3;
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
  //FastLED.addLeds<LPD8806, 12>(leds, LED_COUNT);
  FastLED.addLeds<WS2812, 12, GRB>(leds, LED_COUNT);
  randomSeed (analogRead(0));
  idleAnim.hue = random (255);
  directionAnim.hue = random (255);
  for (uint16_t i = 0; i < LED_COUNT; i++) {
    leds[i] = CHSV (0, 0, 0);
  }
  FastLED.show();
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
