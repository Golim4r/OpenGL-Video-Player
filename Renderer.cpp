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

Renderer* current_renderer;

void redraw_global() {
  //std::cout << "drawing\n";
	current_renderer->frame_data = current_renderer->_dec.get_video_frame();
  
  for (int i=0; i<current_renderer->get_num_windows(); ++i) {
		glutSetWindow(current_renderer->get_window_id(i));
    glFlush();
    
    int & window_width = 
      current_renderer->_windows[i].window_width;
    int & window_height = 
      current_renderer->_windows[i].window_height;

    if (glutGet(GLUT_WINDOW_WIDTH)  != window_width ||
        glutGet(GLUT_WINDOW_HEIGHT) != window_height) {
      window_width  = glutGet(GLUT_WINDOW_WIDTH);
      window_height = glutGet(GLUT_WINDOW_HEIGHT);


      double current_ap = static_cast<double>(window_width) / 
                          static_cast<double>(window_height);
      double desired_ap = 
        static_cast<double>(current_renderer->_dec.get_width()) / 
        static_cast<double>(current_renderer->_dec.get_height());

      std::cout << "aspect ratio: " << window_width << 
                   ":" << window_height << '\n';


      std::cout << "ap: " << current_ap;
      std::cout << " should be: " << desired_ap << '\n';



      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, current_renderer->_dec.get_width(), current_renderer->_dec.get_height(), GL_RGB, GL_UNSIGNED_BYTE, current_renderer->frame_data);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glutSwapBuffers();
	}
}

void keyboard_global(unsigned char key, int x, int y) {
  if( key == 'q' || key == 27 ) {
    current_renderer->_dec.stop();
    glutLeaveMainLoop();
  }
}



GLWindow::GLWindow(int video_width, int video_height, float section_top, float section_bottom, float section_left, float section_right) : 
    _video_width(video_width), _video_height(video_height) {
  std::cout << "trying to create a window" << std::endl;

  vertices[6]   = section_top;
  vertices[13]  = section_top;
  vertices[20]  = section_bottom;
  vertices[27]  = section_bottom;
  vertices[5]   = section_left;
  vertices[26]  = section_left;
  vertices[12]  = section_right;
  vertices[19]  = section_right;
  
  std::cout << "noch da1?\n";  
  glutInitDisplayMode(GLUT_RGB);
  std::cout << "noch da1.1?\n";  
	glutInitWindowSize(400,500);		// width=400pixels height=500pixels
  std::cout << "noch da1.2?\n";  
	id = glutCreateWindow("Triangle");	// create window

  std::cout << "noch da2?\n";  
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
  
  std::cout << "noch da?\n";  

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

	//set pixels array as texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _video_width, _video_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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


/*****************************************************
* RENDERER
*****************************************************/
Renderer::Renderer(Decoder &dec) : _dec(dec) {
  current_renderer = this;
  int argc = 1;
  char* argv[] = {"egal"};
  int win;
	glutInit(&argc, argv);		// initialize GLUT system

  std::cout << "dec width and height: " << _dec.get_width() <<
    ", " << _dec.get_height() << '\n';

  //create an OpenGL window with video size and section sizes , float section_top, float section_bottom, float section_left, float section_right
  _windows.emplace_back(GLWindow(_dec.get_width(), _dec.get_height(), 0.0f, 1.0f, 0.0f, 0.5f));
  _windows.emplace_back(GLWindow(_dec.get_width(), _dec.get_height(), 0.0f, 1.0f, 0.5f, 1.0f));

	//glutMainLoop();
}


void Renderer::run() {
  glutMainLoop();
}

void Renderer::redraw()	{

}

void Renderer::keyboard(unsigned char key, int x, int y) {
  if( key == 'q' || key == 27 ) glutLeaveMainLoop();
}

int Renderer::get_num_windows() {
  return _windows.size();
}

int Renderer::get_window_id(int win) {
  return _windows[win].id;
}



