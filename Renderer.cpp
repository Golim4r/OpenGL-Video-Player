#include "Renderer.h"

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

float pixels[] = {
  0.0f, 0.0f, 0.0f,  1.0f, 0.4f, 0.2f,
  0.3f, 0.6f, 0.9f,  0.0f, 1.0f, 0.0f
};

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
  
GLWindow::GLWindow(int video_width, int video_height) : _video_width(video_width), _video_height(video_height) {
  //std::cout << "trying to create a window" << std::endl;
  _id = glutCreateWindow("testWindow");
  
  glewExperimental = GL_TRUE; //needed as it is old!
  GLenum err = glewInit();
  if (GLEW_OK != err)	{
    std::cerr<<"Error: " << glewGetErrorString(err)<<std::endl;
  } else {
    if (GLEW_VERSION_3_3)
    {
      std::cout<<"Driver supports OpenGL 3.3:"<<std::endl;
    }
  }
  
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  
  //GLuint vbo;
  glGenBuffers(1, &vbo);
  
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  
  glGenBuffers(1, &ebo);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
  
  // Create and compile the vertex shader
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexSource, NULL);
  glCompileShader(vertexShader);
  
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
  glCompileShader(fragmentShader);
  
  // Link the vertex and fragment shader into a shader program
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glBindFragDataLocation(shaderProgram, 0, "outColor");
  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);
  
  // Specify the layout of the vertex data
  posAttrib = glGetAttribLocation(shaderProgram, "position");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);
  
  colAttrib = glGetAttribLocation(shaderProgram, "color");
  glEnableVertexAttribArray(colAttrib);
  glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
  
  texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
  glEnableVertexAttribArray(texAttrib);
  glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));
  
  // Load texture
  tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  
  //set pixels array as texture
  //while(!written[0]) {} //wait for the first frame to be buffered
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pCodecCtx->width, pCodecCtx->height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffered_frames[0].pFrameRGB->data[0]);
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _video_width, _video_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, pixels);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  std::cout << "window created" << std::endl;
}

GLWindow::~GLWindow() {
	glDeleteTextures(1, &tex);

	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

Renderer* current_renderer;



void redraw_global() {
  std::cout << "drawing...\n";
  //glFlush();
  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, current_renderer->_dec.get_width(), current_renderer->_dec.get_height(), GL_RGB, GL_UNSIGNED_BYTE, current_renderer->_dec.get_frame());
  //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, pixels.data());
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	//glutSwapBuffers();
  
  /*
  glColor3f(1.0, 1.0, 1.0);
  glBegin(GL_POLYGON);
  glVertex2i(200,125);
  glVertex2i(100,375);
  glVertex2i(300,375);
  glEnd();
  glFlush();
  glutSwapBuffers();*/
  
  //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  //glClear(GL_COLOR_BUFFER_BIT);

  // Draw a rectangle from the 2 triangles using 6 indices
  std::cout << "----- 1 -----\n";
  glDrawElements(GL_TRIANGLES, 6, GL_FLOAT, 0);

  // Swap buffers
  std::cout << "----- 2 -----\n";
  glutSwapBuffers();
  glFlush();
  std::cout << "----- 3 -----\n";
}

void keyboard_global(unsigned char key, int x, int y) {
  if( key == 'q' || key == 27 ) glutLeaveMainLoop();
}

Renderer::Renderer(Decoder &dec) : _dec(dec) {
  int argc = 1;
  char* argv[] = {"egal"};
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB);
	
  // Set closing behaviour: If we leave the mainloop (e.g. through user input or closing the window) we continue after the function "glutMainLoop()"
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_GLUTMAINLOOP_RETURNS);
  
  _windows.emplace_back(GLWindow(_dec.get_width(), _dec.get_height()));

  glClearColor(0.0,0.0,0.0,0.0);
  glClear(GL_COLOR_BUFFER_BIT);
  glutDisplayFunc(redraw_global);
  glutIdleFunc(redraw_global);
  glutKeyboardFunc(keyboard_global);
}

void Renderer::run() {
  current_renderer = this;
  glutMainLoop();
}

void Renderer::redraw()	{
	//std::cout << "display 1" << std::endl;
	/*std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	if (written[nextFrame]) {
		std::cout << "reading frame " << nextFrame << std::endl;
		//TODO TIMED EVENT
		//rt1 = std::chrono::system_clock::now();
		++framesread;
		
		//SaveFrame(buffered_av_frames[nextFrame], pCodecCtx->width, pCodecCtx->height, nextFrame*10+8);
		for (int i=0; i<WINDOW_COUNT; ++i) {
			glutSetWindow(win_ids[i]);
			glFlush();
			//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pCodecCtx->width, pCodecCtx->height, GL_RGB, GL_UNSIGNED_BYTE, buffered_av_frames[nextFrame]->data[0]);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pCodecCtx->width, pCodecCtx->height, GL_RGB, GL_UNSIGNED_BYTE, buffered_frames[nextFrame].pFrameRGB->data[0]);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glutSwapBuffers();
		}
		
		written[nextFrame] = false;
		++nextFrame;
		//if (nextFrame > 100) { glutLeaveMainLoop(); }
		nextFrame = nextFrame%BUFFERED_FRAMES_COUNT;
		//rt2 = std::chrono::system_clock::now();
		//std::cout << "waited " << notreadycount << " rounds for this frame" << std::endl;
		notreadycount = 0;
	} else {
		++notreadycount;
		//std::cout << "waiting for next frame, " << framesread << " already read, frame " << nextFrame << " is not ready for the " << notreadycount << " time" << std::endl;
	}
	//std::chrono::duration<double> elapsed_seconds = rt2-rt1;
	//std::cout << "elapsed time:" << elapsed_seconds.count() << std::endl;
	//glutSwapBuffers();
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));*/
}

void Renderer::keyboard(unsigned char key, int x, int y) {
  if( key == 'q' || key == 27 ) glutLeaveMainLoop();
}




