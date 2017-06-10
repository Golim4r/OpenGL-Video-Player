#include <iostream>
#include <thread>

#include "Renderer.h"
#include "Decoder.h"
#include "Pulseplayer.h"
#include "JUtils.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

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


void audiothread(Decoder & d) {
  Pulseplayer pp(d.get_sample_rate(), d.get_channels());
  
  while (!d.done) {
    //std::cout << "trying to get an audio frame\n"; 
    pp.play(d.get_audio_frame().data);
  }
}

int main(int argc, char *argv[]) {
  GLVconfig conf = read_config("glv.conf");

  std::string filename;  
  if (argc > 1) {
    filename = argv[1];
	} else {
    std::cerr << "ERROR: no media file specified! terminating...\n";
    return -1;
  }

  Decoder d(filename, conf.decode_video, conf.decode_audio, conf.sync_internally, conf.audiotrack);
  Renderer r(d,conf.windows);
  
  JDurationManager dm;

  dm.start();

  std::thread dt(&Decoder::run, &d);
  std::thread at(audiothread, std::ref(d));
  r.run();

  at.join();
  dt.join();  

  dm.stop();
  dm.print();

	return 0;
}
