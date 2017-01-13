all:
	g++ main.cpp -o main -std=c++11 -L/usr/ri62viw/lib -lavdevice -lavformat -lavfilter -lavcodec -lswscale -lavutil -lGL -lGLU -lglut -lGLEW -lpthread
