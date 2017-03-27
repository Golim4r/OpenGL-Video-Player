#include <iostream>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <vector>
#include <cmath>
#include "Decoder.h"

class Pulseplayer{
private:
  pa_simple *s = NULL;
  pa_sample_spec ss;

  int error;
public:
  Pulseplayer();
  Pulseplayer(int sample_rate, int channels);
  ~Pulseplayer();

  void play(std::vector<uint8_t> samples);
};
