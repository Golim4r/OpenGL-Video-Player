#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include <string>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>

class Audioplayer{
private:
  ALboolean enumeration;
	const ALCchar *devices;
	const ALCchar *defaultDeviceName;
	int ret;
	char *bufferData;
	ALCdevice *device;
	ALvoid *data;
	ALCcontext *context;
	ALsizei size, freq;
	ALenum format;
	ALuint buffer, source;
	ALfloat listenerOri[6] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
	ALboolean loop = AL_FALSE;
	ALCenum error;
	ALint source_state;
public:
  Audioplayer();
  ~Audioplayer();
};