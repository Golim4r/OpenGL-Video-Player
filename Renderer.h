#ifndef RENDERER_H
#define RENDERER_H

#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include <thread>

#include "Decoder.h"





/*
GLfloat vertices[] = {
//  Position      Color             Texcoords
    -1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top-left
     1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.0f, // Top-right
     1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f, // Bottom-right
    -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.5f  // Bottom-left
};

GLuint elements[] = {
    0, 1, 2,
    2, 3, 0
};

class GLWindowStuff {
public:
	GLuint vao, vbo, tex, ebo, vertexShader, fragmentShader, shaderProgram;
	GLint posAttrib, colAttrib, texAttrib;

	void cleanup() {
		std::cout << "trying to cleanup winstuff" << std::endl;
		glDeleteTextures(1, &tex);

		glDeleteProgram(shaderProgram);
		glDeleteShader(fragmentShader);
		glDeleteShader(vertexShader);

		glDeleteBuffers(1, &ebo);
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}
};
*/

class GLWindow{
private:
  int _id, _video_width, _video_height;
  GLuint vao, vbo, tex, ebo, vertexShader, fragmentShader, shaderProgram;
	GLint posAttrib, colAttrib, texAttrib;
  
  //std::vector<GLfloat> vertices = {
  /*GLfloat vertices[28] = {
  //  Position      Color             Texcoords
      -1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top-left
       1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Top-right
       1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
      -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
  };
  
  GLuint elements[6] = {
      0, 1, 2,
      2, 3, 0
  };*/
  
public:
  GLWindow(int video_width, int video_height);
  ~GLWindow();
};

class Renderer {
private:
  /*std::vector<int> win_ids;
  std::vector<AVFrame*> buffered_av_frames(BUFFERED_FRAMES_COUNT);
  std::vector<Buffered_frame> buffered_frames(BUFFERED_FRAMES_COUNT);
  std::vector<std::atomic<bool>> written(BUFFERED_FRAMES_COUNT); // true, if ready to read, false if ready to write
  std::atomic<bool> terminated, ready_to_render;
  
  std::vector<GLWindowStuff> win_stuff;
  std::chrono::time_point<std::chrono::system_clock> rt1, rt2, ct1, ct2;

  long framesread = 0;
  int nextFrame = 0;
  int notreadycount = 0;
  int winid = 0;
  
  void createWindow();
  void initGLUT();*/
  void keyboard(unsigned char key, int x, int y);
  void redraw();

  std::vector<GLWindow> _windows;
  
  //Decoder& _dec;
public:
  Renderer(Decoder &dec);
  //Renderer() {
  //  glutInit(&argc, argv);		// initialize GLUT system
  //}
  void run();
  Decoder& _dec;
};

#endif