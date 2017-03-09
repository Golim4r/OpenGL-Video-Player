#include "Audioplayer.h"

#define TEST_ERROR(_msg)		\
	error = alGetError();		\
	if (error != AL_NO_ERROR) {	\
		std::cout << _msg << ", " << error << " ohohoh\n";	\
	}
  
static void list_audio_devices(const ALCchar *devices)
{
	const ALCchar *device = devices, *next = devices + 1;
	size_t len = 0;

	//fprintf(stdout, "Devices list:\n");
  std::cout << "Devices list:\n";
	//fprintf(stdout, "----------\n");
  std::cout << "-------------\n";
	while (device && *device != '\0' && next && *next != '\0') {
		//fprintf(stdout, "%s\n", device);
    std::cout << device << '\n';
		len = strlen(device);
		device += (len + 1);
		next += (len + 2);
	}
	//fprintf(stdout, "----------\n");
  std::cout << "-------------\n";
}

Audioplayer::Audioplayer() {
	enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
	if (enumeration == AL_FALSE) {
    std::cout << "enumeration extension not available\n";
  }

	list_audio_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
  
	if (!defaultDeviceName) {
		defaultDeviceName = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
  }
  
	device = alcOpenDevice(defaultDeviceName);
	if (!device) {
		//fprintf(stderr, "unable to open default device\n");
    std::cout << "ohoh\n";
	}

	alGetError();
  
	context = alcCreateContext(device, NULL);
	if (!alcMakeContextCurrent(context)) {
		//fprintf(stderr, "failed to make default context\n");
    std::cout << "ohoh2\n";
	}
	TEST_ERROR("make default context");
  
	/* set orientation */
	alListener3f(AL_POSITION, 0, 0, 1.0f);
	TEST_ERROR("listener position");
  alListener3f(AL_VELOCITY, 0, 0, 0);
	TEST_ERROR("listener velocity");
	alListenerfv(AL_ORIENTATION, listenerOri);
	TEST_ERROR("listener orientation");

	alGenSources((ALuint)1, &source);
	TEST_ERROR("source generation");

	alSourcef(source, AL_PITCH, 1);
	TEST_ERROR("source pitch");
	alSourcef(source, AL_GAIN, 1);
	TEST_ERROR("source gain");
	alSource3f(source, AL_POSITION, 0, 0, 0);
	TEST_ERROR("source position");
	alSource3f(source, AL_VELOCITY, 0, 0, 0);
	TEST_ERROR("source velocity");
	alSourcei(source, AL_LOOPING, AL_FALSE);
	TEST_ERROR("source looping");

	alGenBuffers(1, &buffer);
	TEST_ERROR("buffer generation");


  float freq = 440.f;
	int seconds = 4;
	unsigned sample_rate = 22050;
	size_t buf_size = seconds * sample_rate;


  
  short *samples;
  samples = new short[buf_size];
  for(int i=0; i<buf_size; ++i) {
      samples[i] = 32760 * std::sin( (2.f*float(M_PI)*freq)/sample_rate * i );
  }


  alBufferData(buffer, AL_FORMAT_MONO16, samples, buf_size, sample_rate);
  TEST_ERROR("buffer copy");

  alSourcei(source, AL_BUFFER, buffer);
  TEST_ERROR("buffer binding");
  
	alSourcePlay(source);
	TEST_ERROR("source playing");
  
	alGetSourcei(source, AL_SOURCE_STATE, &source_state);
	TEST_ERROR("source state get");
	while (source_state == AL_PLAYING) {
		alGetSourcei(source, AL_SOURCE_STATE, &source_state);
		TEST_ERROR("source state get");
	}
}

Audioplayer::~Audioplayer() {
  /* exit context */
  alDeleteSources(1, &source);
	alDeleteBuffers(1, &buffer);
	device = alcGetContextsDevice(context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

void Audioplayer::play() {
  //alBufferData(buffer, AL_FORMAT_MONO8, data, size, freq);
  //TEST_ERROR("buffer copy");

  /*alSourcei(source, AL_BUFFER, buffer);
  TEST_ERROR("buffer binding");*/
  
	alSourcePlay(source);
	TEST_ERROR("source playing");
  
	alGetSourcei(source, AL_SOURCE_STATE, &source_state);
	TEST_ERROR("source state get");
	while (source_state == AL_PLAYING) {
		alGetSourcei(source, AL_SOURCE_STATE, &source_state);
		TEST_ERROR("source state get");
	}
}


void Audioplayer::play(short* buf) {
  float freq = 300.f;
	int seconds = 4;
	unsigned sample_rate = 22050;
	size_t buf_size = seconds * sample_rate;

  short *samples;
  samples = new short[buf_size];
  for(int i=0; i<buf_size; ++i) {
      samples[i] = 32760 * std::sin( (2.f*float(M_PI)*freq)/sample_rate * i );
  }


  alBufferData(buffer, AL_FORMAT_MONO16, samples, buf_size, sample_rate);
  TEST_ERROR("buffer copy");

	alSourcePlay(source);
	TEST_ERROR("source playing");

  alSourcei(source, AL_BUFFER, buffer);
  TEST_ERROR("buffer binding");
  
	alSourcePlay(source);
	TEST_ERROR("source playing");
  
	alGetSourcei(source, AL_SOURCE_STATE, &source_state);
	TEST_ERROR("source state get");
	while (source_state == AL_PLAYING) {
		alGetSourcei(source, AL_SOURCE_STATE, &source_state);
		TEST_ERROR("source state get");
	}
}
