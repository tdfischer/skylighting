#include "animation.h"

Animation::Animation(LPD8806* strip, int frames, int framerate) :
  m_strip (strip),
  m_frames (frames),
  m_framerate (framerate),
  m_finished (false)
{
}

void
Animation::step()
{
  m_idx += m_framerate;
  if (m_frames > 0 && m_idx >= m_frames)
    reset();
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
  m_finished = false;
}


LPD8806&
Animation::strip() const
{
  return *m_strip;
}

bool
Animation::isFinished() const
{
  return m_finished;
}

void
Animation::end()
{
  m_finished = true;
}
