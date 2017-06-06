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
  Pulseplayer(uint32_t sample_rate, uint8_t channels);
  ~Pulseplayer();

  void play(std::vector<uint8_t> samples);
};
