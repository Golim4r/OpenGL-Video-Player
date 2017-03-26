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

template <typename T>
class Buffer {
private:
  std::vector<T> _data;
  std::vector<std::atomic<bool>> _occupied;
  int _size, _read_position, _write_position;
  T temp;
public:
  Buffer(int size) : 
    _data(size), 
    _occupied(size), 
    _size(size), 
    _read_position(0), 
    _write_position(0) {}

  void put(T elem) {
    //todo: not busy wait
    while (_occupied[_write_position]) {}
    _data[_write_position] = std::move(elem);
    _occupied[_write_position] = true;
    _write_position = ++_write_position % _size;
  }

  const T & get() {
    //todo: not busy wait
    while (!_occupied[_read_position]) { }
    temp = std::move(_data[_read_position]);
    _occupied[_read_position] = false;
    _read_position = ++_read_position % _size;
    return temp;
  }
};

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

  bool has_audio = false;
  
  std::vector<std::vector<uint8_t>> buffered_video_frames;
  std::vector<std::vector<uint8_t>> buffered_audio_frames;
  std::vector<uint8_t> buffer_riesen_audio;  
  
  std::vector<uint8_t> audio_frame;
  Buffer<std::vector<uint8_t>> audio_frames;  

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

  const bool & get_done();
};

#endif
