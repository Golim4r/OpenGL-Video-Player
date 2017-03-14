#include "Pulseplayer.h"

Pulseplayer::Pulseplayer() {
  ss = { .format = PA_SAMPLE_S16LE,
         .rate = 44100,
         .channels = 1 };
  std::cout << "do we have a simple spec? hopefully, cant check\n";
}

Pulseplayer::~Pulseplayer() { 
  if (s) pa_simple_free(s);  
}
