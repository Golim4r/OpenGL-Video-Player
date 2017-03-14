all:
	g++ main.cpp Decoder.cpp Renderer.cpp Pulseplayer.cpp -o main -std=c++11 -lavdevice -lavformat -lavfilter -lavcodec -lswscale -lavutil -lGL -lGLU -lglut -lGLEW -lpthread -lpulse-simple -lpulse
#-lopenal -lalut -O0
