#include <iostream>
#include <thread>

#include "Renderer.h"
#include "Decoder.h"
//#include "Audioplayer.h"
#include "Pulseplayer.h"
#include "JUtils.h"


void audiothread(Decoder & d) {
  Pulseplayer asd;
  //asd.play();
  //std::cout << "decoder bla: " << d.get_width() << '\n';
  asd.play(d.get_audio_frame_test());
  asd.play(d.get_audio_frame_test());
  asd.play(d.get_audio_frame_test());
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
