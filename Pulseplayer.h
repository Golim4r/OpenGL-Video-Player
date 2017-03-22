#include <iostream>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <vector>
#include <cmath>

class Pulseplayer{
private:
  pa_simple *s = NULL;
  pa_sample_spec ss;

  int error;
public:
  Pulseplayer();
  ~Pulseplayer();

  void play();
  void play(std::vector<short> samples);
  void play(std::vector<uint8_t> samples);
};
