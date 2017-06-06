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
    pp.play(d.get_audio_frame().data);
  }
}

int main(int argc, char *argv[]) {
  std::string filename = "CarRace.mp4";

  if (argc > 1) {
    filename = argv[1];
	}

  Decoder d(filename, true, true, false);
  //Decoder d(filename);

  Renderer r(d);
  JDurationManager dm;

  dm.start();

  std::thread dt(&Decoder::run, &d);
  std::thread at(audiothread, std::ref(d));
  //r.run();
  

  /*for (int i=0; i<100; ++i) {
    MediaFrame cvf = d.get_video_frame();
    //std::cout << "frame: " << cvf.pts << '\n';

    r.draw(cvf.data);
  }
  d.seek(60);
  for (int i=0; i<100; ++i) {
    MediaFrame cvf = d.get_video_frame();
    //std::cout << "frame: " << cvf.pts << '\n';

    r.draw(cvf.data);
  }
  d.seek(-3);

  JIpsManager ips;
  while(!d.done) {
    ips.update(true);
    MediaFrame cvf = d.get_video_frame();
    //std::cout << "frame: " << cvf.pts << '\n';

    r.draw(cvf.data);
    
    
    //r.draw(d.get_video_frame().data);
    //glutSwapBuffers();
  }
*/

  r.run();

  at.join();
  dt.join();  

  dm.stop();
  dm.print();

	return 0;
}
