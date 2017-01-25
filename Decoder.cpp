#include "Decoder.h"

Decoder::Decoder(std::string filename) {
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
    //initialize things for buffering
    
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
  for (int i=0; i<20; ++i) {
    while (!buffer_frame()) { }
    SaveFrame(i);
    //buffer and pFrameRGB->data[0] are the same
    //std::cout << "pFrameRGB[0]: " << &pFrameRGB->data[0][1] << '\n';
    //std::cout << "buffer: " << &buffer[1] << '\n';
    
  }
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
 
bool Decoder::buffer_frame() {
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
        //SaveFrame(0);
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
