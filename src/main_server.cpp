#include <config.h>
#include <iostream>
#include <thread>

#include <renderServer.h>
#include <synchObject.h>

//#include "Renderer.h"
#include "Pulseplayer.h"
#include "Decoder.h"
#include "JUtils.h"

void audiothread(Decoder & d) {
  Pulseplayer pp(d.get_sample_rate(), d.get_channels());
  
  while(!d.done) {
    pp.play(d.get_audio_frame().data);
  }
}

int main(int argc, char** argv) {
  //synchlib::caveConfig a;
  //std::string file("default.cfg");
  //a.writeDefault(file);

  //config file
  std::string cfile = argv[1];

  //path to media file
  std::string mediafile = 
    //argc > 2 ? argv[2] : 
		  "/home/di73yad/opengl_videoplayer/media/Max.mp4";
  
  //current video frame
  MediaFrame current_video_frame;

  //create the decoder with video, audio and internal synching
  Decoder d(mediafile);

  //create the renderer
  //Renderer r(d);

  //start the decoder and audio threads
  std::thread dt(&Decoder::run, &d);
  std::thread at(audiothread, std::ref(d));

  //create the synch server  
  synchlib::renderServer server(cfile, argc, argv);

  //timestamp
  std::shared_ptr<synchlib::SynchObject<size_t>> video_time_stamp = 
    synchlib::SynchObject<size_t>::create();

  server.addSynchObject(video_time_stamp,synchlib::renderServer::SENDER,0);

  server.init(true);
  server.startSynching();

  while (!d.done) {
    //current_video_frame = d.get_video_frame();
    //video_time_stamp->setData(current_video_frame.pts);
    video_time_stamp->setData(d.get_video_frame().pts);
    video_time_stamp->send();

  }

  server.stopSynching();

  dt.join();
  at.join();
}
