#include <config.h>
#include <iostream>
#include <thread>

#include <renderServer.h>
#include <synchObject.h>

#include "Pulseplayer.h"
#include "Renderer.h"
#include "Decoder.h"
#include "JUtils.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

void audiothread(Decoder & d) {
  Pulseplayer pp(d.get_sample_rate(), d.get_channels());
  
  while(!d.done) {
    pp.play(d.get_audio_frame().data);
  }
}

struct GLVconfig {
  bool decode_audio, decode_video, sync_internally;
  int audiotrack;
  std::vector<WindowConfig> windows;
};

GLVconfig read_config(std::string filename) {
  GLVconfig conf;
  boost::property_tree::ptree tree;
  boost::property_tree::read_xml(filename, tree);

  conf.decode_audio = tree.get<bool>("play_audio");
  conf.decode_video = tree.get<bool>("play_video");
  conf.sync_internally = tree.get<bool>("sync_internally");
  conf.audiotrack = tree.get<int>("audiotrack");

  for (auto & s : tree.get_child("screens")) {
    WindowConfig wc;
    wc.name = s.second.get<std::string>("<xmlattr>.name");
    wc.x_begin = s.second.get<float>("x_begin");
    wc.x_end   = s.second.get<float>("x_end");
    wc.y_begin = s.second.get<float>("y_begin");
    wc.y_end = s.second.get<float>("y_end");
    
    std::cout << "second: " << s.second.get<std::string>("<xmlattr>.name")<< '\n';
    std::cout << "x_begin: " << s.second.get<float>("x_begin");
    std::cout << "x_end: " << s.second.get<float>("x_end");
    conf.windows.push_back(wc);
  }
  
  return conf;
}

int main(int argc, char** argv) {
  //synchlib::caveConfig a;
  //std::string file("default.cfg");
  //a.writeDefault(file);

 auto c = read_config("glv.conf");   

  //config file
  std::string cfile = argv[1];

  //path to media file
  std::string mediafile = 
    argc > 2 ? argv[2] : 
	  "media/Max.mp4";
  
  //current video frame
  MediaFrame current_video_frame;

  //create the decoder with video, audio and internal synching
  Decoder d(mediafile);

  //create the renderer
  //Renderer r(d);

  //start the decoder and audio threads
  std::thread dt(&Decoder::run, &d);
  std::thread at(audiothread, std::ref(d));

  std::cout << "noch da -1\n";

  //create the synch server  
  synchlib::renderServer server(cfile, argc, argv);

  std::cout << "noch da?0\n";
  
  //timestamp
  std::shared_ptr<synchlib::SynchObject<size_t>> video_time_stamp = 
    synchlib::SynchObject<size_t>::create();


  std::cout << "noch da?1\n";
  server.addSynchObject(video_time_stamp,synchlib::renderServer::SENDER,0);

  std::cout << "noch da?2\n";
  server.init(true);
  std::cout << "noch da?3\n";
  server.startSynching();

  std::cout << "noch da? final\n";
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
