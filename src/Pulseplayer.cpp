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

Pulseplayer::Pulseplayer(uint32_t sample_rate, uint8_t channels) {
  ss = { .format = PA_SAMPLE_U8,
         .rate = sample_rate,
         .channels = channels };
  std::cout << "sample rate: " << sample_rate << "channels: " << static_cast<int>(channels) << '\n';
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
