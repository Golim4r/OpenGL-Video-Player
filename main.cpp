#include <iostream>
#include <thread>

#include "Renderer.h"
#include "Decoder.h"
//#include "Audioplayer.h"
#include "Pulseplayer.h"
#include "JUtils.h"

void audiothread(Decoder & d) {
  Pulseplayer pp(d.get_sample_rate(), d.get_channels());
  //Audioplayer asd;
  //JTimedIterationManager pp_tim(d.tim, 1.f / 44100.f);
  //asd.play();
  //asd.play(d.get_sine_audio_frame());
  
  while(true) {
  //while (!d.done) {
    pp.play(d.get_audio_frame());
    if (d.done) { break; }
  }
  std::cout << "audiothread is done...\n";
}

int main(int argc, char *argv[]) {
  std::string filename = "CarRace.mp4";

  if (argc > 1) {
    filename = argv[1];
	}

  Decoder d(filename);

  Renderer r(d);
  JDurationManager dm;

  dm.start();

  std::thread dt(&Decoder::run, &d);
  std::thread ap(audiothread, std::ref(d));
  r.run();

  ap.join();
  dt.join();  

  dm.stop();
  dm.print();

	return 0;
}
