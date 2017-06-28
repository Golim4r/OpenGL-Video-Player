#include <iostream>
#include <thread>

#include <renderNode.h>
#include <synchObject.h>

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
    WindowConfig wc(
    s.second.get<std::string>("<xmlattr>.name"),
    s.second.get<float>("x_begin"),
    s.second.get<float>("x_end"),
    s.second.get<float>("y_begin"),
    s.second.get<float>("y_end"));
    
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
    pp.play(d.get_audio_frame().data);
  }
}


int main(int argc, char *argv[]) {
  std::string cfile = argv[2];

  std::string glv_config_path;
  if (argc < 5) {
    std::cout << "WARNING: no glv config specified, trying to read default\n";
    glv_config_path = "/home/jakob/Documents/workspaces/OpenGL-Video-Player/glv.conf";
  } else {
    glv_config_path = argv[5];
  }
  auto conf = read_config(glv_config_path);

  std::vector<WindowConfig> c;
  std::string own_name = std::string(argv[1]) + "_" + std::string(argv[3]);
  for (auto & w : conf.windows) {
    if (w.name == own_name) {
      c.push_back(w);
    }
  }

  if (c.size() == 0) {
    std::cerr << "I don't have a window config... goodbye\n";
    return -1;
  }

  std::string mediafile = argv[4];

  //decoder with video, no audio, and no internal synching
  Decoder d(mediafile, true, false, false);

  Renderer r(d, c);
  
  
  
  
  
  
  synchlib::renderNode node(cfile,argc,argv);

  std::shared_ptr<synchlib::SynchObject<size_t>> video_syncher =
    synchlib::SynchObject<size_t>::create();
  node.addSynchObject(video_syncher,synchlib::renderNode::RECEIVER);


  std::shared_ptr<synchlib::SynchObject<size_t>> seek_timer =
    synchlib::SynchObject<size_t>::create();
  node.addSynchObject(seek_timer,synchlib::renderNode::RECEIVER);
  
  std::shared_ptr<synchlib::SynchObject<int>> control =
    synchlib::SynchObject<int>::create();

  node.init();
  node.startSynching();
  
  
  
  




  //start the decoder
  std::thread dt(&Decoder::run, &d);
  
  
  MediaFrame current_video_frame = d.get_video_frame();
  size_t current_pts;
 
  //r.set_fullscreen();
 
  int test = 0;
  while (!d.done) {
    if (seek_timer->hasChanged()) {
      size_t seeky;
      seek_timer->getData(seeky);
      d.seek(seeky);
    }
    video_syncher->getData(current_pts);
    current_video_frame = d.get_video_frame();

    //if too early, wait
    while(current_pts < current_video_frame.pts){
      if(!node.synchFrame()) {break;}
      if (seek_timer->hasChanged()) {
        size_t seeky;
        seek_timer->getData(seeky);
        d.seek(seeky);
      }
      video_syncher->getData(current_pts);
      //std::cout<<"wait for server"<< ++test%10 <<   std::endl;
      usleep(1000);
    }
    
    //if too late, get next frame
    while (current_pts > current_video_frame.pts) {
      current_video_frame = d.get_video_frame();

    }
    //if on time, synch with the others
    r.draw(current_video_frame.data);
    if(!node.synchFrame()) {break;}
    glutSwapBuffers();
    node.synchFrame(); 
  }
  
  d.stop();
  dt.join();

	return 0;
}
