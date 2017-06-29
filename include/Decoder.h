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

#define BUFFERED_FRAMES_COUNT 9

struct MediaFrame {
  MediaFrame(){}
  MediaFrame(uint8_t *data_ptr, size_t data_size, size_t timestamp):
    data(data_ptr, data_ptr + data_size), pts(timestamp) {}

  size_t pts;
  std::vector<uint8_t> data;
};

template <typename T>
class Buffer {
private:
  std::vector<T> _data;
  std::vector<std::atomic<bool>> _occupied;
  int _size, _read_position, _write_position;
  T temp;
  std::atomic<bool> _done;
public:
  Buffer(int size) : 
    _data(size), 
    _occupied(size), 
    _size(size), 
    _read_position(0), 
    _write_position(0),
    _done(false) {}

  void put(T elem) {
    while (_occupied[_write_position]) {
      std::cout << "cant write at " << _write_position << "\n";
      if (_done) { return; }
    }
    //std::cout << "writing at " << _write_position << '\n';
    _data[_write_position] = std::move(elem);
    _occupied[_write_position] = true;
    _write_position = ++_write_position % _size;
  }

  const T & get() {
    while (!_occupied[_read_position]) { 
      std::cout << "cant read at"<< _read_position << '\n';
      if (_done) {
        temp = T(); 
        return temp; 
      }
    }
    //std::cout << "reading at " << _read_position << '\n';
    temp = std::move(_data[_read_position]);
    _occupied[_read_position] = false;
    _read_position = ++_read_position % _size;
    return temp;
  }

  void clear() {
    for (int i=0; i<_data.size(); ++i) {
      _occupied[i] = false;
      _data[i] = T();
    }
    temp = T();
    _read_position = 0;
    _write_position = 0;
  }

  void stop() {
    _done = true;
  }

  void resume() {
    _done = false;
  }
};

class Decoder {
private:
  AVFormatContext   *pFormatCtx = NULL;
  int               videoStream;
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
  double            aspect_ratio;

  std::vector<int> audioStreams;
  std::atomic<int> currentAudioStream;

  std::atomic<bool> _sync_local;

  std::atomic<bool> _seeking;

  MediaFrame videoframe;
  Buffer<MediaFrame> vframes;
  MediaFrame audioframe;
  Buffer<MediaFrame> aframes;

  size_t current_video_pts;
  std::atomic<int> _seek_seconds;

  void SaveFrame(int iFrame);
  bool read_frame();
  void seek();

  JTimedIterationManager vtim, atim;
  
  bool _decodeVideo, _decodeAudio;
public:
  Decoder() = delete;
  Decoder(std::string filename, 
      bool decodeVideo=true, bool decodeAudio=true, bool sync_local = true,
      int audio_stream=0);
  ~Decoder();
  
  std::atomic<bool> done;

  void run();
  void stop();  
  void seek(const size_t & video_frame_pts);
  //void next_audio_stream();
  //void previous_audio_stream();

  MediaFrame get_video_frame();
  MediaFrame get_audio_frame();
  
  const int & get_width();
  const int & get_height();
  const double & get_aspect_ratio();

  int get_sample_rate();
  int get_channels();
};

#endif
