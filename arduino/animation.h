#include <LPD8806.h>

class Animation {
public:
  Animation(LPD8806* strip);

  virtual void redraw() = 0;
  void step();
  int idx() const;
  void reset();

  LPD8806& strip() const;

private:
  LPD8806* m_strip;
  int m_idx;
};


