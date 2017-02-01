#include "Decoder.h"

Decoder::Decoder(std::string filename) : current_frame_reading(0), current_frame_writing(0), written(BUFFERED_FRAMES_COUNT), tim(106) {
  try {
    std::cout << "opening file " << filename << std::endl;
    av_register_all();
  
    // Open video file
    if(avformat_open_input(&pFormatCtx, filename.c_str(), NULL, NULL)!=0) {
      throw 0;
    }
  
    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0) {
      throw 1;
    }
  
    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, filename.c_str(), 0);
  
    // Find the first video stream
    videoStream=-1;
    for(int i=0; i<pFormatCtx->nb_streams; i++) {
      if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
          videoStream=i;
          break;
        }
    }
    if(videoStream==-1) {
      throw 2;
    }
  
    // Get a pointer to the codec context for the video stream
    pCodecCtxOrig=pFormatCtx->streams[videoStream]->codec;
    fps = static_cast<double>(pFormatCtx->streams[videoStream]->avg_frame_rate.num) / static_cast<double>(pFormatCtx->streams[videoStream]->avg_frame_rate.den);
    //std::cout << "---- fps num: " << pFormatCtx->streams[videoStream]->avg_frame_rate.num << ", denum: " << pFormatCtx->streams[videoStream]->avg_frame_rate.den << '\n';
    //std::cout << "----FPS: " << fps << "----\n";
    tim.set_interval(1000.f/fps);
    // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(pCodecCtxOrig->codec_id);
    if(pCodec==NULL) {
      throw 3;
    }
    // Copy context
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if(avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0) {
      throw 4;
    }
  
    // Open codec
    if(avcodec_open2(pCodecCtx, pCodec, NULL)<0) {
      throw 5;
    }
    
    // Allocate video frame
    pFrame=av_frame_alloc();
  
    // Allocate an AVFrame structure
    pFrameRGB=av_frame_alloc();
    if(pFrameRGB==NULL) {
      throw 6;
    }
    
    // Determine required buffer size and allocate buffer
    numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
    buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
  
    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
  
    // initialize SWS context for software scaling
    sws_ctx = sws_getContext(	pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                  PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
    
    //initialize the buffer vector
    buffered_frames.resize(BUFFERED_FRAMES_COUNT);
    std::for_each(buffered_frames.begin(), buffered_frames.end(), [=](std::vector<uint8_t> &v) { v.resize(numBytes); });    
  } catch (int e) {
    std::vector<std::string> error_strings = 
    { "Error opening file\n",
      "Error reading stream information\n",
      "no videostream found\n",
      "unsupported codec\n",
      "Couldn't copy codec context\n",
      "Couldn't open codec\n",
      "Couldn't allocate pFrameRGB\n"};
    std::cerr << "EXCEPTION:\n" << error_strings[e];
  }
}

Decoder::~Decoder() {
	// Free the RGB image
	av_free(buffer);
	av_frame_free(&pFrameRGB);
	
	// Free the YUV frame
	av_frame_free(&pFrame);
  
	// Close the codecs
	avcodec_close(pCodecCtx);
	avcodec_close(pCodecCtxOrig);

	// Close the video file
	avformat_close_input(&pFormatCtx);
 }
 
void Decoder::run() {
  std::cout << "Decoder trying to run!\n";
  done = false;
  tim.start();
  //for (int i=0; i<BUFFERED_FRAMES_COUNT; ++i) {
  while(!done) {
    for (int i=0; i<BUFFERED_FRAMES_COUNT; ++i) {
      //wait for a completed frame
      while (!read_frame()) {
        if (done) break;
      }
      while (written[current_frame_writing]) {
        if (done) break;
      }
      
      //copy the read frame into buffered_frames
      std::memcpy(buffered_frames[i].data(), buffer, numBytes);
      written[current_frame_writing] = true;
      current_frame_writing = (current_frame_writing + 1) % BUFFERED_FRAMES_COUNT;
    }
  }
}

void Decoder::stop() {
  for (int i=0; i<BUFFERED_FRAMES_COUNT; ++i) {
    written[i] = false;
  }
  done = true;
}
 
//void Decoder::SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
void Decoder::SaveFrame(int iFrame) {
  std::cout << "saving frame " << iFrame << '\n';
  FILE *pFile;
  char szFilename[32];
  int  y;
  
  // Open file
  sprintf(szFilename, "frame%d.ppm", iFrame);
  pFile=fopen(szFilename, "wb");
  if(pFile==NULL)
    return;
  
  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", pCodecCtx->width, pCodecCtx->height);
  
  // Write pixel data
  for(y=0; y<pCodecCtx->height; y++)
    fwrite(pFrameRGB->data[0]+y*pFrameRGB->linesize[0], 1, pCodecCtx->width*3, pFile);
  
  // Close file
  fclose(pFile);
}
 
bool Decoder::read_frame() {
  bool frameComplete = false;
  if(av_read_frame(pFormatCtx, &packet)>=0) {
    // Is this a packet from the video stream?
    if(packet.stream_index==videoStream) {
      // Decode video frame
      avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
      
      // Did we get a video frame?
      if(frameFinished) {
        // Convert the image from its native format to RGB
        sws_scale(	sws_ctx, (uint8_t const * const *)pFrame->data,
              pFrame->linesize, 0, pCodecCtx->height,
              pFrameRGB->data,pFrameRGB->linesize);
        frameComplete = true;
        //std::cout << "wrote frame " << i << '\n';
      } else {
        //std::cout << "no finished frame" << std::endl;
      }
    } else {
      //std::cout << "not a video packet" << std::endl;
    }
    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
  }
  return frameComplete;
}

uint8_t* Decoder::get_frame(){
  tim.wait();
  if (written[current_frame_reading]) {
    clear_frame_for_writing();
    return buffered_frames[current_frame_reading].data();
  }
  if (current_frame_reading == 0) {
    return buffered_frames[BUFFERED_FRAMES_COUNT-1].data();
  }
  return buffered_frames[current_frame_reading-1].data();
}

void Decoder::clear_frame_for_writing() {
  written[current_frame_reading] = false;
  current_frame_reading = (current_frame_reading + 1) % BUFFERED_FRAMES_COUNT;
}

int Decoder::get_width() {
  return pCodecCtx->width;
}

int Decoder::get_height() {
  return pCodecCtx->height;
}