#include "Renderer.h"

void Renderer::createWindow() {
  //std::cout << "trying to create a window" << std::endl;
  //GLWindowStuff stuff;
  win_stuff.push_back(GLWindowStuff());
  int id = win_stuff.size()-1;
  win_ids.push_back(glutCreateWindow("testWindow"));
  
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
  
  glGenVertexArrays(1, &win_stuff[id].vao);
  glBindVertexArray(win_stuff[id].vao);
  
  //GLuint vbo;
  glGenBuffers(1, &win_stuff[id].vbo);
  
  glBindBuffer(GL_ARRAY_BUFFER, win_stuff[id].vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  
  glGenBuffers(1, &win_stuff[id].ebo);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, win_stuff[id].ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
  
  // Create and compile the vertex shader
  win_stuff[id].vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(win_stuff[id].vertexShader, 1, &vertexSource, NULL);
  glCompileShader(win_stuff[id].vertexShader);
  
  win_stuff[id].fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(win_stuff[id].fragmentShader, 1, &fragmentSource, NULL);
  glCompileShader(win_stuff[id].fragmentShader);
  
  // Link the vertex and fragment shader into a shader program
  win_stuff[id].shaderProgram = glCreateProgram();
  glAttachShader(win_stuff[id].shaderProgram, win_stuff[id].vertexShader);
  glAttachShader(win_stuff[id].shaderProgram, win_stuff[id].fragmentShader);
  glBindFragDataLocation(win_stuff[id].shaderProgram, 0, "outColor");
  glLinkProgram(win_stuff[id].shaderProgram);
  glUseProgram(win_stuff[id].shaderProgram);
  
  // Specify the layout of the vertex data
  win_stuff[id].posAttrib = glGetAttribLocation(win_stuff[id].shaderProgram, "position");
  glEnableVertexAttribArray(win_stuff[id].posAttrib);
  glVertexAttribPointer(win_stuff[id].posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);
  
  win_stuff[id].colAttrib = glGetAttribLocation(win_stuff[id].shaderProgram, "color");
  glEnableVertexAttribArray(win_stuff[id].colAttrib);
  glVertexAttribPointer(win_stuff[id].colAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
  
  win_stuff[id].texAttrib = glGetAttribLocation(win_stuff[id].shaderProgram, "texcoord");
  glEnableVertexAttribArray(win_stuff[id].texAttrib);
  glVertexAttribPointer(win_stuff[id].texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));
  
  // Load texture
  win_stuff[id].tex;
  glGenTextures(1, &win_stuff[id].tex);
  glBindTexture(GL_TEXTURE_2D, win_stuff[id].tex);
  
  //set pixels array as texture
  while(!written[0]) {} //wait for the first frame to be buffered
  SaveFrame(buffered_frames[0].pFrameRGB, pCodecCtx->width, pCodecCtx->height, 0+win_ids.size());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pCodecCtx->width, pCodecCtx->height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffered_frames[0].pFrameRGB->data[0]);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  std::cout << "window created" << std::endl;
}

void Renderer::redraw()		// function called whenever redisplay needed
{
	//std::cout << "display 1" << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
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
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void keyboard(unsigned char key, int x, int y) {
  if( key == 'q' || key == 27 ) glutLeaveMainLoop();
}

void Renderer::initGLUT() {
	glutInitDisplayMode(GLUT_RGB);
	
    // Set closing behaviour: If we leave the mainloop (e.g. through user input or closing the window) we continue after the function "glutMainLoop()"
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_GLUTMAINLOOP_RETURNS);


	/*************************************
	* Main Loop
	*************************************/
	for (int i=0; i<WINDOW_COUNT; ++i) {
		createWindow();
	}
	
	glClearColor(0.0,0.0,0.0,0.0);	// set background to black
    gluOrtho2D(0,400,0,500);		// how object is mapped to window
    glutDisplayFunc(displayCB);		// set window's display callback
    glutIdleFunc(displayCB);
    glutKeyboardFunc(keyCB);		// set window's key callback

	glutMainLoop();
	
	terminated = true;
	
	for (int i=0; i<win_stuff.size(); ++i) {
		win_stuff[i].cleanup();
	}
}