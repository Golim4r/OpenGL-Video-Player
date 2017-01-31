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

std::vector<uint8_t> pixels_test(6220800);


Renderer* current_renderer;



void redraw_global() {
  //std::cout << "drawing...\n";
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
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
  // Draw a rectangle from the 2 triangles using 6 indices

  //std::cout << "0\n";
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, pixels);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, current_renderer->_dec.get_width(), current_renderer->_dec.get_height(), 0, GL_RGB, GL_UNSIGNED_BYTE, current_renderer->_dec.get_frame().data());
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGB, GL_FLOAT, pixels);
  //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, current_renderer->_dec.get_width(), current_renderer->_dec.get_height(), GL_RGB, GL_UNSIGNED_BYTE, current_renderer->_dec.get_frame().data());

  //std::cout << "1\n";
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  //std::cout << "2\n";
  glutSwapBuffers();
}

void keyboard_global(unsigned char key, int x, int y) {
  if( key == 'q' || key == 27 ) glutLeaveMainLoop();
}

GLWindow::GLWindow(int video_width, int video_height) : 
    _video_width(video_width), _video_height(video_height) {
  //std::cout << "trying to create a window" << std::endl;

  glutInitDisplayMode(GLUT_RGB);
	glutInitWindowSize(400,500);		// width=400pixels height=500pixels
	_id = glutCreateWindow("Triangle");	// create window

	// Initialise glew
	glewExperimental = GL_TRUE; //needed as it is old!
	GLenum err = glewInit();
	if (GLEW_OK != err)	{
	   std::cerr<<"Error: "<<glewGetErrorString(err)<<std::endl;
	} else {
	   if (GLEW_VERSION_3_3)
	   {
		  std::cout<<"Driver supports OpenGL 3.3:"<<std::endl;
	   }
	}

  glClearColor(0.0,0.0,0.0,0.0);	// set background to black
  gluOrtho2D(0,400,0,500);		// how object is mapped to window
  glutDisplayFunc(redraw_global);		// set window's display callback
  glutIdleFunc(redraw_global);
  glutKeyboardFunc(keyboard_global);		// set window's key callback

  // Set closing behaviour: If we leave the mainloop (e.g. through user input or closing the window) we continue after the function "glutMainLoop()"
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_GLUTMAINLOOP_RETURNS);


	// Create Vertex Array Object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create a Vertex Buffer Object and copy the vertex data to it
	GLuint vbo;
	glGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Create an element array
	GLuint ebo;
	glGenBuffers(1, &ebo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	// Create and compile the vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	// Create and compile the fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	// Link the vertex and fragment shader into a shader program
	GLuint shaderProgram = glCreateProgram();
	//shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	// Specify the layout of the vertex data
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	//posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);

	GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
	//colAttrib = glGetAttribLocation(shaderProgram, "color");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

	GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
	//texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));

	// Load texture
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);

	//set pixels array as texture current_renderer->_dec.get_width(), current_renderer->_dec.get_height(), GL_RGB, GL_UNSIGNED_BYTE, current_renderer->_dec.get_frame());
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, pixels);
  std::cout << "before texture setting\n";
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _video_width, _video_height, 0, GL_RGB, GL_UNSIGNED_BYTE, current_renderer->_dec.get_frame().data());
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _video_width, _video_height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels_test.data());
  std::cout << "after texture setting\n";
  
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  //std::cout << "window created" << std::endl;
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

Renderer::Renderer(Decoder &dec) : _dec(dec) {
  current_renderer = this;
  int argc = 1;
  char* argv[] = {"egal"};
  int win;
	glutInit(&argc, argv);		// initialize GLUT system

  _windows.emplace_back(GLWindow(_dec.get_width(), _dec.get_height()));

	//glutMainLoop();
}

void Renderer::run() {
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




