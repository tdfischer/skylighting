#include <stdint.h>

class Animation {
public:
  Animation() :
    m_finished (false)
  {}

  virtual void redraw() = 0;
  virtual void step()
  {
    m_idx++;
    redraw();
  }

  uint8_t idx() const {return m_idx;}
  void reset()
  {
    m_idx = 0;
    m_finished = false;
  }
  bool isFinished() const {return m_finished;}

protected:
  uint8_t m_idx;
  bool m_finished;
};

