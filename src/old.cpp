
void printAudioFrameInfo(const AVCodecContext* codecContext, const AVFrame* frame)
{
    // See the following to know what data type (unsigned char, short, float, etc) to use to access the audio data:
    // http://ffmpeg.org/doxygen/trunk/samplefmt_8h.html#af9a51ca15301871723577c730b5865c5
    std::cout << "Audio frame info:\n"
              << "  Sample count: " << frame->nb_samples << '\n'
              << "  Channel count: " << codecContext->channels << '\n'
              << "  Format: " << av_get_sample_fmt_name(codecContext->sample_fmt) << '\n'
              << "  Bytes per sample: " << av_get_bytes_per_sample(codecContext->sample_fmt) << '\n'
              << "  Is planar? " << av_sample_fmt_is_planar(codecContext->sample_fmt) << '\n';

    std::cout << "audio format: " << codecContext->sample_fmt << '\n';
    std::cout << "frame->linesize[0] tells you the size (in bytes) of each plane\n";

    if (codecContext->channels > AV_NUM_DATA_POINTERS && av_sample_fmt_is_planar(codecContext->sample_fmt))
    {
        std::cout << "The audio stream (and its frames) have too many channels to fit in\n"
                  << "frame->data. Therefore, to access the audio data, you need to use\n"
                  << "frame->extended_data to access the audio data. It's planar, so\n"
                  << "each channel is in a different element. That is:\n"
                  << "  frame->extended_data[0] has the data for channel 1\n"
                  << "  frame->extended_data[1] has the data for channel 2\n"
                  << "  etc.\n";
    }
    else
    {
        std::cout << "Either the audio data is not planar, or there is enough room in\n"
                  << "frame->data to store all the channels, so you can either use\n"
                  << "frame->data or frame->extended_data to access the audio data (they\n"
                  << "should just point to the same data).\n";
    }

    std::cout << "If the frame is planar, each channel is in a different element.\n"
              << "That is:\n"
              << "  frame->data[0]/frame->extended_data[0] has the data for channel 1\n"
              << "  frame->data[1]/frame->extended_data[1] has the data for channel 2\n"
              << "  etc.\n";

    std::cout << "If the frame is packed (not planar), then all the data is in\n"
              << "frame->data[0]/frame->extended_data[0] (kind of like how some\n"
              << "image formats have RGB pixels packed together, rather than storing\n"
              << " the red, green, and blue channels separately in different arrays.\n";
}
