#ifndef RENDERER_H
#define RENDERER_H

#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include <thread>
#include <fstream>

#include "Decoder.h"


class GLWindow{
private:
  int _video_width, _video_height;
  GLuint vao, vbo, tex, ebo, vertexShader, fragmentShader, shaderProgram;
	GLint posAttrib, colAttrib, texAttrib;
  
  //std::vector<GLfloat> vertices = {

  GLfloat vertices[28] = {
  //  Position      Color             Texcoords
      -1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top-left
       1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Top-right
       1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
      -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
  };
  
  GLuint elements[6] = {
      0, 1, 2,
      2, 3, 0
  };
  
  float desired_aspect_ratio;
  bool bound_top, bound_bottom, bound_left, bound_right;
public:
  GLWindow(int video_width, int video_height, float section_top, float section_bottom, float section_left, float section_right);
  ~GLWindow();

  int id;

  void update_section();
  int window_width, window_height;
};

class Renderer {
private:
  void keyboard(unsigned char key, int x, int y);

  //Decoder& _dec;
public:
  Renderer(Decoder &dec);

  void run();
  //should be private, but redraw must be global?
  Decoder& _dec;
  //uint8_t* frame_data;

  void redraw();

  std::vector<GLWindow> _windows;
  
  int get_num_windows();
  int get_window_id(int win);
};

#endif
