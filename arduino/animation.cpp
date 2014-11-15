#include "animation.h"

Animation::Animation(LPD8806* strip) :
  m_strip (strip)
{
}

void
Animation::step()
{
  m_idx++;
  redraw();
}

int
Animation::idx() const
{
  return m_idx;
}

void
Animation::reset()
{
  m_idx = 0;
  redraw();
}


LPD8806&
Animation::strip() const
{
  return *m_strip;
}
