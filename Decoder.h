#ifndef DECODER_H
#define DECODER_H

#ifdef __cplusplus
extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}
#endif

#include <string>
#include <iostream>
#include <vector>

const int BUFFERED_FRAMES_COUNT = 120;
const int WINDOW_COUNT = 5;

class Decoder {
private:
  AVFormatContext   *pFormatCtx = NULL;
  int               videoStream;
  AVCodecContext    *pCodecCtxOrig = NULL;
  AVCodecContext    *pCodecCtx = NULL;
  AVCodec           *pCodec = NULL;
  AVFrame           *pFrame = NULL;
  AVFrame           *pFrameRGB = NULL;
  AVPacket          packet;
  int               frameFinished;
  int               numBytes;
  uint8_t           *buffer = NULL;
  struct SwsContext *sws_ctx = NULL;
  
  long frameswritten = 0;
  bool terminated;
  
  //void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame);
  void SaveFrame(int iFrame);
  bool buffer_frame();
public:
  Decoder() = delete;
  Decoder(std::string filename = "CarRace.mp4");
  ~Decoder();
  
  void run();
};

#endif