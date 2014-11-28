#include <LPD8806.h>

class Animation {
public:
  Animation(LPD8806* strip, int frames = 0, int framerate = 1);

  virtual void redraw() = 0;
  void step();
  int idx() const;
  void reset();
  void end();
  bool isFinished() const;
  int frames() const;

  LPD8806& strip() const;

private:
  LPD8806* m_strip;
  int m_frames;
  int m_idx;
  int m_framerate;
  bool m_finished;
};


