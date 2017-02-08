#include <iostream>
#include <thread>

#include "Renderer.h"
#include "Decoder.h"
#include "Audioplayer.h"
#include "JUtils.h"


int main(int argc, char *argv[]) {
  std::string filename = "CarRace.mp4";
	if (argc > 1) {
		filename = argv[1];
	}
  Audioplayer asd;
  asd.play();
  
  Decoder d(filename);
  
  Renderer r(d);
  JDurationManager dm;
  
  dm.start();
  //d.run();
  std::thread dt(&Decoder::run, &d);
  r.run();
  dt.join();  
  
  asd.play(d.get_audio_frame());
  
  dm.stop();
  dm.print();
  
	return 0;
}
