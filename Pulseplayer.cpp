#include "Pulseplayer.h"

Pulseplayer::Pulseplayer() {
  ss = { .format = PA_SAMPLE_U8,
         .rate = 44100,
         .channels = 2 };
  std::cout << "do we have a simple spec? hopefully, cant check\n";

  if(!(s = pa_simple_new(NULL, "test",PA_STREAM_PLAYBACK, NULL, 
      "playback", &ss, NULL, NULL, &error))) {
    std::cout << "no we dont\n";
  }
}

Pulseplayer::Pulseplayer(int sample_rate, int channels) {
  ss = { .format = PA_SAMPLE_U8,
         .rate = sample_rate,
         .channels = channels };

  if(!(s = pa_simple_new(NULL, "test",PA_STREAM_PLAYBACK, NULL, 
      "playback", &ss, NULL, NULL, &error))) {
    std::cout << "no we dont\n";
  }
}

Pulseplayer::~Pulseplayer() { 
  if (s) pa_simple_free(s);  
  std::cout << "Pulseplayer is destroyed\n";
}

void Pulseplayer::play(std::vector<uint8_t> samples) {
  if (samples.size() == 0) { return; }
  pa_simple_write(s, samples.data(), samples.size(), &error);
}


/*void Pulseplayer::play() {
  
  float freq = 440.f;
  int seconds = 5;
  unsigned int sample_rate = 44100;
  size_t buf_size = seconds * sample_rate;

  std::vector<short> samples(buf_size);
  for (int i=0; i<buf_size; ++i) {
    samples[i] = 32760 * std::sin((2.f*float(M_PI)*freq)
                 /sample_rate*i);
  }
  if (pa_simple_write(s, samples.data(), samples.size(), &error)
      < 0) {
    std::cerr << "nunja, wohl nicht\n";
  }
  if (pa_simple_drain(s, &error) < 0) {
    std::cerr << "drain nicht\n";
  }
}*/
