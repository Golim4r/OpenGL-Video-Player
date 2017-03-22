#all:
#	g++ main.cpp Decoder.cpp Renderer.cpp Pulseplayer.cpp -o main -std=c++11 -lavdevice -lavformat -lavfilter -lavcodec -lswscale -lavutil -lGL -lGLU -lglut -lGLEW -lpthread -lpulse-simple -lpulse
#-lopenal -lalut -O0
CPPFLAGS=-std=c++11 -lpthread
FFMPEGFLAGS=-lavdevice -lavformat -lavfilter -lavcodec -lswscale -lavutil -lswresample
GLFLAGS=-lGL -lGLU -lglut -lGLEW
ALFLAGS=-lopenal -lalut
PULSEFLAGS=-lpulse-simple -lpulse

all: glv

glv: main.o Decoder.o Renderer.o Pulseplayer.o
	g++ main.o Decoder.o Renderer.o Pulseplayer.o -o glv $(CPPFLAGS) $(PULSEFLAGS) $(GLFLAGS) $(FFMPEGFLAGS)

main.o: main.cpp
	g++ -c main.cpp $(CPPFLAGS)

Decoder.o: Decoder.cpp
	g++ -c Decoder.cpp $(CPPFLAGS) $(FFMPEGFLAGS)

Renderer.o: Renderer.cpp
	g++ -c Renderer.cpp $(CPPFLAGS) $(GLFLAGS)

Pulseplayer.o: Pulseplayer.cpp
	g++ -c Pulseplayer.cpp $(CPPFLAGS) $(PULSEFLAGS)

openal: 
	g++ main.cpp Decoder.cpp Renderer.cpp Audioplayer.cpp -o glval -std=c++11 -lavdevice -lavformat -lavfilter -lavcodec -lswscale -lavutil -lGL -lGLU -lglut -lGLEW -lpthread -lalut -lopenal
 

clean:
	rm *o glv
