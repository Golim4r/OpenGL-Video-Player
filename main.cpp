#include <iostream>
#include <thread>

#include "Renderer.h"
#include "Decoder.h"
#include "Pulseplayer.h"
#include "JUtils.h"

void audiothread(Decoder & d) {
  Pulseplayer pp(d.get_sample_rate(), d.get_channels());
  
  while (!d.done) {
    //std::cout << "trying to get an audio frame\n"; 
    pp.play(d.get_audio_frame());
  }
}

int main(int argc, char *argv[]) {
  std::string filename = "CarRace.mp4";

  if (argc > 1) {
    filename = argv[1];
	}

  Decoder d(filename, true, false, true);
  //Decoder d(filename);

  Renderer r(d);
  JDurationManager dm;

  dm.start();

  std::thread dt(&Decoder::run, &d);
  std::thread at(audiothread, std::ref(d));
  //r.run();
  
  while(!d.done) {
    r.draw(d.get_video_frame());
  }

  at.join();
  dt.join();  

  dm.stop();
  dm.print();

	return 0;
}
