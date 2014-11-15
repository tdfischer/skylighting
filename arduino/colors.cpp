#include "colors.h"

uint32_t wheel(uint16_t wheelPos, LPD8806& strip)
{
  byte r, g, b;
  switch(wheelPos / 128)
  {
    case 0:
      r = 127 - wheelPos % 128;   //Red down
      g = wheelPos % 128;      // Green up
      b = 0;                  //blue off
      break; 
    case 1:
      g = 127 - wheelPos % 128;  //green down
      b = wheelPos % 128;      //blue up
      r = 0;                  //red off
      break; 
    case 2:
      b = 127 - wheelPos % 128;  //blue down 
      r = wheelPos % 128;      //red up
      g = 0;                  //green off
      break; 
  }
  return(strip.Color(r, g, b));
}
