#include <config.h>
#include <iostream>
#include <thread>

#include <renderServer.h>
#include <synchObject.h>

#include "Pulseplayer.h"
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

std::vector<std::pair<std::string,std::string>> read_config(const std::string & filename) {
  boost::property_tree::ptree tree;
  boost::property_tree::read_xml(filename, tree);

  std::vector<std::pair<std::string,std::string>> config;
  std::vector<std::string> names, sections;

  for( auto & p : tree.get_child("names")) {
    names.push_back(p.second.data());
  }

  for (auto & p : tree.get_child("sections")) {
    sections.push_back(p.second.data());
  }

  if (sections.size() != names.size()) {
    return config;
  }

  for (int i=0; i<names.size(); ++i) {
    config.push_back(std::make_pair(names[i], sections[i]));
  }

  return config;
}

int main(int argc, char** argv) {
  //synchlib::caveConfig a;
  //std::string file("default.cfg");
  //a.writeDefault(file);

  for (auto & c : read_config("sections.conf")) {
    std::cout << c.first << ": " << c.second << '\n';
  }

  //config file
  std::string cfile = argv[1];

  //path to media file
  std::string mediafile = 
    argc > 2 ? argv[2] : 
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
