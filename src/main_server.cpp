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

std::atomic<size_t> global_seek;
std::atomic<int> global_control;

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
  
  while(!d.done) {
    pp.play(d.get_audio_frame().data);
  }
}

void keyboard(unsigned char key, int x, int y) {
  std::cout << "keypress!\n";
  switch (key) {
    case 27:
    case 'q':
      global_control = -1;
      glutLeaveMainLoop();
      break;
    case 's':
      global_seek = 10;
      break;
    case 'S':
      global_seek = -10;
      break;
  }
}

int main(int argc, char** argv) {
  global_seek = 0;
  global_control = 0;


  if (argc < 3) {
    std::cout << "USAGE:\n";
    std::cout << "parameter 0: [executable]\n";
    std::cout << "parameter 1: absolute path to synchlib config file\n";
    std::cout << "parameter 2: absolute path to glv config file\n";
    std::cout << "parameter 3: absolute path to media file\n";
  }

  //synchlib config file
  std::string cfile = argv[1];

  //glv config file
  auto c = read_config(argv[2]);   

  //media file
  std::string mediafile = argv[3];

  //current video frame
  MediaFrame current_video_frame;

  //create the decoder with video, audio and internal synching
  Decoder d(mediafile);

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB);
  glutInitWindowSize(200,200);
  glutCreateWindow("Control Window");

  glClearColor(0,0,0,1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glFlush();
  glSwapBuffers();
  glutKeyboardFunc(keyboard);

  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_GLUTMAINLOOP_RETURNS);

  //start the decoder and audio threads
  std::thread dt(&Decoder::run, &d);
  std::thread at(audiothread, std::ref(d));


  //create the synch server  
  synchlib::renderServer server(cfile, argc, argv);
  
  //timestamp
  std::shared_ptr<synchlib::SynchObject<size_t>> video_time_stamp = 
    synchlib::SynchObject<size_t>::create();
  server.addSynchObject(video_time_stamp,synchlib::renderServer::SENDER,0);

  std::shared_ptr<synchlib::SynchObject<size_t>> seek_time =
    synchlib::SynchObject<size_t>::create();
  server.addSynchObject(seek_time, synchlib::renderServer::SENDER,0);

  std::shared_ptr<synchlib::SynchObject<int>> control =
    synchlib::SynchObject<int>::create();
  server.addSynchObject(control, synchlib::renderServer::SENDER,0);

  server.init(true);
  server.startSynching();

  std::thread syncer([&]() {
    while (!d.done) {
      if (global_seek != 0) {
        d.seek(global_seek);
        seek_time->setData(global_seek);
        seek_time->send();
        global_seek = 0;
      }
      
      if (global_control == -1) {
        break;
      }

      control->setData(global_control);
      control->send();

      video_time_stamp->setData(d.get_video_frame().pts);
      video_time_stamp->send();
    }
  });

  glutMainLoop();
  d.stop();

  dt.join();
  at.join();
  syncer.join();

  server.stopSynching();
}
