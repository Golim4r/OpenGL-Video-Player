#include <iostream>
#include <pulse/simple.h>
#include <pulse/error.h>

class Pulseplayer{
private:
  pa_simple *s = NULL;
  pa_sample_spec ss;

  int error;
public:
  Pulseplayer();
  ~Pulseplayer();
};
