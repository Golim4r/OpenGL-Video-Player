#include <iostream>
#include <thread>

#include <renderNode.h>
#include <synchObject.h>

#include "Renderer.h"
#include "Decoder.h"
#include "Pulseplayer.h"
#include "JUtils.h"

void audiothread(Decoder & d) {
  Pulseplayer pp(d.get_sample_rate(), d.get_channels());
  
  while (!d.done) {
    pp.play(d.get_audio_frame().data);
  }
}

int main(int argc, char *argv[]) {
  std::string cfile = argv[2];

  //does this have to be a pointer?
  synchlib::renderNode node(cfile,argc,argv);

  std::shared_ptr<synchlib::SynchObject<size_t>> video_syncher =
    synchlib::SynchObject<size_t>::create();

  node.addSynchObject(video_syncher,synchlib::renderNode::RECEIVER);

  node.init();
  node.startSynching();
  
  
  
  std::string mediafile = 
    //argc > 1 ? argv[1] : 
  "/home/di73yad/opengl_videoplayer/media/Max.mp4";

  //decoder with video, no audio, and no internal synching
  Decoder d(mediafile, true, false, false);
  //Decoder d(filename);

  Renderer r(d);
  



  //JDurationManager dm;
  //dm.start();

  //start the decoder
  std::thread dt(&Decoder::run, &d);
  //std::thread at(audiothread, std::ref(d));
  
  //r.run(); //uses glutmainloop
  
  MediaFrame current_video_frame = d.get_video_frame();
  size_t current_pts;
  r.set_fullscreen();
  while (!d.done) {
    //current_video_frame = d.get_video_frame();
    
    //while (!video_syncher->hasChanged()) {}
    video_syncher->getData(current_pts);
    current_video_frame = d.get_video_frame();

    while(current_pts < current_video_frame.pts){
         video_syncher->getData(current_pts);
         std::cout<<"wait for server"<<std::endl;
         usleep(1000);
    }
    //if too late, just draw
    while (current_pts > current_video_frame.pts) {
      //r.draw(current_video_frame.data);
      current_video_frame = d.get_video_frame();

    }
    //if on time, synch with the others
//    if (current_pts == current_video_frame.pts) {
      r.draw(current_video_frame.data);
      if(!node.synchFrame())
    	  break;
      glutSwapBuffers();
      node.synchFrame(); 
    //  current_video_frame = d.get_video_frame();
//    }

  }
d.stop();
  //at.join();
  dt.join();
  std::cout<<"exit"<<std::endl;

  //dm.stop();
  //dm.print();

	return 0;
}
