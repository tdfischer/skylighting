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
#define LED_COUNT 28
CRGB leds[LED_COUNT];

static void fadeLEDSegment(uint16_t idx, CRGB goal, int width)
{
  CRGB start = leds[idx];
  for(fract8 i = 0; i < 255; i++) {
    CRGB blended = blend(start, goal, ease8InOutCubic(i));
    for (uint16_t segmentIdx = idx; segmentIdx <= idx + width; segmentIdx++) {
      leds[segmentIdx] = blended;
    }
    FastLED.show();
    delay(10);
  }
}

static void fadeLED(uint16_t idx, CRGB goal)
{
  return fadeLEDSegment(idx, goal, 0);
}


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
    hue = 0;
    for (uint16_t  i = 0; i < LED_COUNT; i++) {
      leds[i] = CHSV(hue, 255, 255 * ((pos - i)/(double)LED_COUNT));
    }
    FastLED.show();
    delay(30);
  }
};

class TwinkleAnimation : public Animation {
public:
  TwinkleAnimation() :
    Animation(),
    render_func(&TwinkleAnimation::do_fill),
    nextColor(0, 255, 255)
  {}

  void (TwinkleAnimation::*render_func)();
  CHSV nextColor;
  uint16_t iterations;

  void redraw()
  {
    (this->*render_func)();
  }

  void do_fill()
  {
    for (uint16_t i = 0; i < LED_COUNT; i++) {
      leds[i] = CRGB::White;
    }
    FastLED.show();
    fadeLEDSegment(0, CRGB::Black, LED_COUNT);
    nextColor = CHSV(random(0, 255), 255, 255);
    render_func = &TwinkleAnimation::do_random_shift;
  }

  void do_random_shift()
  {
    uint16_t pixelIdx = random(0, LED_COUNT);
    fadeLED(pixelIdx, nextColor);
    iterations++;
    if (iterations == 100) {
      nextColor.h += random(20, 30);
      nextColor.s = max(nextColor.s+random(10, 30), 100);
      iterations = 0;
    }
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
  }
};

DropAnimation idleAnim;
HouselightsAnimation houselightsAnim;
DirectionAnimation directionAnim;
TwinkleAnimation twinkleAnim;
Animation* anim (&twinkleAnim);

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
  FastLED.addLeds<LPD8806, GRB>(leds, LED_COUNT);
  //FastLED.addLeds<WS2812, 12, GRB>(leds, LED_COUNT);
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
