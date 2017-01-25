#ifndef RENDERER_H
#define RENDERER_H

#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

const GLchar* vertexSource =
  "#version 150 core\n"
  "in vec2 position;"
  "in vec3 color;"
  "in vec2 texcoord;"
  "out vec3 Color;"
  "out vec2 Texcoord;"
  "void main()"
  "{"
  "    Color = color;"
  "    Texcoord = texcoord;"
  "    gl_Position = vec4(position, 0.0, 1.0);"
  "}";
const GLchar* fragmentSource =
  "#version 150 core\n"
  "in vec3 Color;"
  "in vec2 Texcoord;"
  "out vec4 outColor;"
  "uniform sampler2D tex;"
  "void main()"
  "{"
  "    outColor = texture(tex, Texcoord);"
  "}";



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

class Renderer {
private:
  std::vector<int> win_ids;
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
  void initGLUT();
  void keyboard();
  void redraw();
public:
  Renderer() {
    glutInit(&argc, argv);		// initialize GLUT system
  }
};

#endif