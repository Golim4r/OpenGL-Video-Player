#include <iostream>
#include <thread>

#include "Renderer.h"
#include "Decoder.h"
//#include "Audioplayer.h"
#include "Pulseplayer.h"
#include "JUtils.h"

void audiothread(Decoder & d) {
  Pulseplayer asd;
  //Audioplayer asd;
  
  //asd.play();
  //asd.play(d.get_sine_audio_frame());
  
  while(true) {
  //for (int i=0; i<100; ++i) {
    asd.play(d.get_audio_frame());
  }
}

int main(int argc, char *argv[]) {
  std::string filename = "CarRace.mp4";
  Buffer<int> buf(15);


	/*std::thread t1([&] {  
		for (int i=0; i<200; ++i) {
			buf.put(i);
		}
	});

	std::thread t2([&] {	    
    while (buf.get() != 199 ) { }
	});
	
  t1.join();
  t2.join();*/
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
