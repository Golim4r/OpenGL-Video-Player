#ifndef DECODER_H
#define DECODER_H

#ifdef __cplusplus
extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
}
#endif

#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "JUtils.h"

#define BUFFERED_FRAMES_COUNT 120

class Decoder {
private:
  AVFormatContext   *pFormatCtx = NULL;
  int               videoStream, audioStream;
  AVCodecContext    *pCodecCtxOrig = NULL;
  AVCodecContext    *pCodecCtx = NULL;
  AVCodecContext    *aCodecCtx = NULL;
  AVCodec           *pCodec = NULL;
  AVFrame           *pFrame = NULL;
  AVFrame           *pFrameRGB = NULL;
  AVFrame           *aFrame = NULL;
  AVPacket          packet;
  int               frameFinished;
  int               numBytes;
  uint8_t           *buffer = NULL;
  struct SwsContext *sws_ctx = NULL;
  struct SwrContext *swr_ctx = NULL;
  double            fps;
  double            aspect_ratio;
  
  //experimental swresample stuff
  
  
  //end of experimental

  bool has_audio = false;
  
  std::vector<std::vector<uint8_t>> buffered_video_frames;
  std::vector<std::vector<short>> buffered_audio_frames;
  std::vector<uint8_t> buffer_riesen_audio;  

  std::vector<uint16_t> audio_test_buffer;
  
  std::atomic<bool> done;
  
  std::atomic<int> current_frame_reading, current_frame_writing;
  std::vector<std::atomic<bool>> written;

  void SaveFrame(int iFrame);
  bool read_frame();


  
  JTimedIterationManager tim;
  
  bool first_time = true;
public:
  Decoder() = delete;
  Decoder(std::string filename = "CarRace.mp4");
  ~Decoder();
  
  void run();
  void stop();
  
  uint8_t* get_video_frame();
  void clear_frame_for_writing();
  
  std::vector<uint8_t> get_sine_audio_frame();
  std::vector<uint8_t> get_audio_frame();
  
  const int & get_width();
  const int & get_height();

  const double & get_aspect_ratio();
};

#endif
