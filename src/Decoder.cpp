#include "Decoder.h"

Decoder::Decoder(std::string filename, 
    bool decodeVideo, bool decodeAudio, bool sync_local, int audio_stream) : 
  vframes(BUFFERED_FRAMES_COUNT),
  aframes(BUFFERED_FRAMES_COUNT),
  vtim(106), atim(107), //random values, are overwritten anyways
  done(false),
  _decodeVideo(decodeVideo),
  _decodeAudio(decodeAudio),
  _sync_local(sync_local),
  _seek_seconds(0),
  current_video_pts(0),
  currentAudioStream(audio_stream)
{
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
    videoStream = -1;
    
    for(int i=0; i<pFormatCtx->nb_streams; i++) {
      if (pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
        videoStream = i;
        //break;
      }
      if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
        audioStreams.push_back(i);
      }
    }
    if(videoStream==-1) {
      throw 2;
    }

    if (audioStreams.size() != 0) {
      if (currentAudioStream >= audioStreams.size()) { currentAudioStream = 0; }

      aCodecCtx = pFormatCtx->streams[audioStreams[currentAudioStream]]->codec;
      aFrame = av_frame_alloc();
	  
      aCodecCtx->codec = avcodec_find_decoder(aCodecCtx->codec_id);
      if (aCodecCtx->codec == NULL) {
        std::cout << "Couldn't find a proper audio decoder" << std::endl;
        throw 7;
        //return 1;
      }
      else if (avcodec_open2(aCodecCtx, aCodecCtx->codec, NULL) != 0) {
        std::cout << "Couldn't open the context with the decoder" << std::endl;
        throw 8;
      }

      //std::cout << "This stream has " << aCodecCtx->channels << 
        //" channels and a sample rate of " << aCodecCtx->sample_rate << 
        //"Hz" << std::endl;
      //std::cout << "The data is in the format " << 
        //av_get_sample_fmt_name(aCodecCtx->sample_fmt) << std::endl;
    } else {
      std::cout << "has no audio stream\n";
    }
  
    // Get a pointer to the codec context for the video stream
    pCodecCtxOrig=pFormatCtx->streams[videoStream]->codec;
    
    vtim.set_interval(av_q2d(pFormatCtx->streams[videoStream]->time_base)*1000);
    atim.set_interval(av_q2d(pFormatCtx->streams[audioStreams[currentAudioStream]]->time_base)*1000);

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
    sws_ctx = sws_getContext(	pCodecCtx->width, pCodecCtx->height,
                  pCodecCtx->pix_fmt, pCodecCtx->width, 
                  pCodecCtx->height, PIX_FMT_RGB24, SWS_BILINEAR,
                  NULL, NULL, NULL);

    swr_ctx = swr_alloc_set_opts(NULL, aCodecCtx->channel_layout,
                  AV_SAMPLE_FMT_U8, aCodecCtx->sample_rate,
                  aCodecCtx->channel_layout, aCodecCtx->sample_fmt,
                  aCodecCtx->sample_rate, 0, NULL);
	  swr_init(swr_ctx);
    
  } catch (int e) {
    std::vector<std::string> error_strings = 
    { "Error opening file\n",
      "Error reading stream information\n",
      "no videostream found\n",
      "unsupported codec\n",
      "Couldn't copy codec context\n",
      "Couldn't open codec\n",
      "Couldn't allocate pFrameRGB\n",
      "Couldn't find a proper audio decoder",
      "Couldn't open the context with the decoder"};
    std::cerr << "EXCEPTION:\n" << error_strings[e];
    
    //todo cleanup
  }
}

Decoder::~Decoder() {
  // Free the audio frame and close codec
  av_frame_free(&aFrame);
  avcodec_close(aCodecCtx);
	
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

  std::cout << "Decoder is destroyed\n";
}
 
void Decoder::run() {
  //std::cout << "Decoder trying to run!\n";
  //done = false;
  vtim.set_start_now();
  atim.set_start(vtim.get_start());
  size_t ctr = 0;
  while(!done) {
    while(read_frame() && !done) { 

      if (_seek_seconds != 0) {
        seek();
        _seek_seconds = 0;
      }
    }
  }
  stop();
}

void Decoder::stop() {
  //done = true;
  //std::cout << "trying to stop decoder\n";
  aframes.stop();
  vframes.stop();
  done = true;
  //std::cout << "decoder stopped\n";
}
 
void Decoder::seek(const size_t & seconds) {
  _seek_seconds = seconds;
}

void Decoder::seek() {
  std::cout << "now seeking...";
  size_t seek_target = current_video_pts + 
    (_seek_seconds / av_q2d(pFormatCtx->streams[videoStream]->time_base));

  av_free_packet(&packet);
  av_freep(&packet);

  av_seek_frame(pFormatCtx, videoStream, seek_target, AVSEEK_FLAG_BACKWARD);
  
  avcodec_flush_buffers(pCodecCtx);
  avcodec_flush_buffers(aCodecCtx);

  //std::this_thread::sleep_for(std::chrono::seconds(1));
  aframes.clear();
  atim.add_offset(-_seek_seconds);
  
  vframes.clear();
  vtim.add_offset(-_seek_seconds);

  std::cout << "done\n";
}


/*void Decoder::next_audio_stream() {
  currentAudioStream = 
    currentAudioStream == audioStreams.size() - 1 ?
    0 : currentAudioStream + 1;
}

void Decoder::previous_audio_stream() {
  currentAudioStream = 
    currentAudioStream == 0 ?
    audioStreams.size() - 1 : currentAudioStream - 1;
}
*/

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
  if (av_read_frame(pFormatCtx, &packet) >= 0) {
    // Is this a packet from the video stream?
    if (packet.stream_index == videoStream && _decodeVideo) {
      // Decode video frame
      avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
      // Did we get a video frame?
      if (frameFinished) {
        // Convert the image from its native format to RGB
        sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
          pFrame->linesize, 0, pCodecCtx->height,
          pFrameRGB->data, pFrameRGB->linesize);

        //std::cout << "putting videoframe " << pFrame->pkt_pts << "\n";
        vframes.put(MediaFrame(buffer, numBytes, pFrame->pkt_pts));
        current_video_pts = pFrame->pkt_pts;
      }
      else {
        //std::cout << "no finished frame" << std::endl;
      }
    }
    else if (packet.stream_index == audioStreams[currentAudioStream] && _decodeAudio) {
      while (packet.size > 0) {
        int gotFrame = 0;
        int result = avcodec_decode_audio4(aCodecCtx, aFrame, &gotFrame, &packet);
        if (result >= 0 && gotFrame) {
          packet.size -= result;
          packet.data += result;

          const uint8_t **in = (const uint8_t **)aFrame->extended_data;
          uint8_t *out = NULL;
          int out_linesize;
          av_samples_alloc(&out,
            &out_linesize,
            2,
            44100,
            AV_SAMPLE_FMT_U8,
            0);

          int ret = swr_convert(swr_ctx,
            &out,
            44100,
            in,
            aFrame->nb_samples);

          aframes.put(MediaFrame(out, ret*aCodecCtx->channels, aFrame->pkt_pts));

          free(out);
        }
        else {
          packet.size = 0;
          packet.data = nullptr;
        }
      }
    }
    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
  } else {
    std::cout << "end of stream?\n";
    done = true;
  }
  return true;
}

MediaFrame Decoder::get_video_frame() {
  if (!_decodeVideo) {
    return MediaFrame();
  }

  videoframe = vframes.get();

  //video_time_stamps.get();
  //video_frames.get();

  //std::cout << "vts: " << video_ts << '\n';
  if (_sync_local) {
    //std::cout<< "vtim: " << vtim.wait(videoframe.pts) << '\n';
    vtim.wait(videoframe.pts);
  }

  //std::cout << "get video frame " << videoframe.pts << " \n";
  return videoframe;
}

MediaFrame Decoder::get_audio_frame() {
  if (!_decodeAudio) {
    return MediaFrame();
  }
  
  audioframe = aframes.get();
  
  if (_sync_local && atim.wait(audioframe.pts)<0) {
    //audioframe.data.resize(audioframe.data.size()-100);
    //std::cout << "audio too late\n";
    //return audioframe;
    return MediaFrame();
  }

  //std::cout << "get audio frame "<<audioframe.pts<<"\n";
  return audioframe;
}

const int & Decoder::get_width() {
  return pCodecCtx->width;
}

const int & Decoder::get_height() {
  return pCodecCtx->height;
}

const double & Decoder::get_aspect_ratio() {
  return aspect_ratio;
}

const int & Decoder::get_sample_rate() {
  return aCodecCtx->sample_rate;
}

const int & Decoder::get_channels() {
  return aCodecCtx->channels;
}
