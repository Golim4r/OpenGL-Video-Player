#define GLEW_STATIC

#ifdef __cplusplus
extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}
#endif

#include <iostream>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>



AVFormatContext   *pFormatCtx = NULL;
int               videoStream;
AVCodecContext    *pCodecCtxOrig = NULL;
AVCodecContext    *pCodecCtx = NULL;
AVCodec           *pCodec = NULL;
AVFrame           *pFrame = NULL;
AVFrame           *pFrameRGB = NULL;
AVPacket          packet;
int               frameFinished;
int               numBytes;
uint8_t           *buffer = NULL;
struct SwsContext *sws_ctx = NULL;

class Buffered_frame {
public:
	AVPacket packet;
	AVFrame* pFrame;
	AVFrame* pFrameRGB;

	Buffered_frame() {
		pFrame = av_frame_alloc();
		pFrameRGB = av_frame_alloc();
	}

	~Buffered_frame() {
		av_frame_free(&pFrame);
		av_frame_free(&pFrameRGB);
	}

	void init() {
		avpicture_fill((AVPicture *)pFrame, buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
		avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
	}
};

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

const int BUFFERED_FRAMES_COUNT = 120;
const int WINDOW_COUNT = 5;

std::vector<AVFrame*> buffered_av_frames(BUFFERED_FRAMES_COUNT);
std::vector<Buffered_frame> buffered_frames(BUFFERED_FRAMES_COUNT);
std::vector<std::atomic<bool>> written(BUFFERED_FRAMES_COUNT); // true, if ready to read, false if ready to write
std::atomic<bool> terminated, ready_to_render;

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

std::vector<int> win_ids;
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

std::vector<GLWindowStuff> win_stuff;
std::chrono::time_point<std::chrono::system_clock> rt1, rt2, ct1, ct2;

long frameswritten = 0, framesread = 0;

void displayCB(void);
void keyCB(unsigned char key, int x, int y);

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
  FILE *pFile;
  char szFilename[32];
  int  y;
  
  // Open file
  sprintf(szFilename, "frame%d.ppm", iFrame);
  pFile=fopen(szFilename, "wb");
  if(pFile==NULL)
    return;
  
  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);
  
  // Write pixel data
  for(y=0; y<height; y++)
    fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
  
  // Close file
  fclose(pFile);
}

bool buffer_frame(int i) {
	bool frameComplete = false;
	if(av_read_frame(pFormatCtx, &buffered_frames[i].packet)>=0) {
		// Is this a packet from the video stream?
		if(buffered_frames[i].packet.stream_index==videoStream) {
			// Decode video frame
			avcodec_decode_video2(pCodecCtx, buffered_frames[i].pFrame, &frameFinished, &buffered_frames[i].packet);
	
			// Did we get a video frame?
			if(frameFinished) {
				// Convert the image from its native format to RGB
				sws_scale(	sws_ctx, (uint8_t const * const *)buffered_frames[i].pFrame->data,
							buffered_frames[i].pFrame->linesize, 0, pCodecCtx->height,
							buffered_frames[i].pFrameRGB->data, buffered_frames[i].pFrameRGB->linesize);
				frameComplete = true;
				//std::cout << "wrote frame " << i << '\n';
			} else {
				//std::cout << "no finished frame" << std::endl;
			}
		} else {
			//std::cout << "not a video packet" << std::endl;
		}
		// Free the packet that was allocated by av_read_frame
		av_free_packet(&buffered_frames[i].packet);
	}
	return frameComplete;
}


bool get_frames_into_av_buffer(int i) {
	bool frameComplete = false;
	if(av_read_frame(pFormatCtx, &packet)>=0) {
		// Is this a packet from the video stream?
		if(packet.stream_index==videoStream) {
			// Decode video frame
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
	
			// Did we get a video frame?
			if(frameFinished) {
				// Convert the image from its native format to RGB
				sws_scale(	sws_ctx, (uint8_t const * const *)pFrame->data,
							pFrame->linesize, 0, pCodecCtx->height,
							buffered_av_frames[i]->data, buffered_av_frames[i]->linesize);
				frameComplete = true;
				//std::cout << "wrote frame " << i << '\n';
			} else {
				//std::cout << "no finished frame" << std::endl;
			}
		} else {
			//std::cout << "not a video packet" << std::endl;
		}
		// Free the packet that was allocated by av_read_frame
		av_free_packet(&packet);
	}
	return frameComplete;
}

void createWindow() {
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

int initFFMPEG(int argc, char* argv[]) {
	std::this_thread::sleep_for(std::chrono::seconds(2));
	std::string filename = "CarRace.mp4";
	if (argc > 1) {
		filename = argv[1];
	}
	
	std::cout << "opening file " << filename << std::endl;
	av_register_all();

	// Open video file
	if(avformat_open_input(&pFormatCtx, filename.c_str(), NULL, NULL)!=0) {
		return -1; // Couldn't open file
	}

	// Retrieve stream information
	if(avformat_find_stream_info(pFormatCtx, NULL)<0) {
		return -1; // Couldn't find stream information
	}

	// Dump information about file onto standard error
	av_dump_format(pFormatCtx, 0, filename.c_str(), 0);

	// Find the first video stream
	videoStream=-1;
	for(int i=0; i<pFormatCtx->nb_streams; i++) {
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
	      videoStream=i;
	      break;
	    }
	}
	if(videoStream==-1) {
		return -1; // Didn't find a video stream
	}

	// Get a pointer to the codec context for the video stream
	pCodecCtxOrig=pFormatCtx->streams[videoStream]->codec;
	// Find the decoder for the video stream
	pCodec=avcodec_find_decoder(pCodecCtxOrig->codec_id);
	if(pCodec==NULL) {
		fprintf(stderr, "Unsupported codec!\n");
		return -1; // Codec not found
	}
	// Copy context
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if(avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0) {
		fprintf(stderr, "Couldn't copy codec context");
		return -1; // Error copying codec context
	}

	// Open codec
	if(avcodec_open2(pCodecCtx, pCodec, NULL)<0) {
		return -1; // Could not open codec
	}
	
	// Allocate video frame
	pFrame=av_frame_alloc();

	// Allocate an AVFrame structure
	pFrameRGB=av_frame_alloc();
	if(pFrameRGB==NULL) {
		return -1;
	}
	
	// Determine required buffer size and allocate buffer
	numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
	buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

	// Assign appropriate parts of buffer to image planes in pFrameRGB
	// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
	// of AVPicture
	avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

	// initialize SWS context for software scaling
	sws_ctx = sws_getContext(	pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
								PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
	//initialize things for buffering
	for (int i=0; i<BUFFERED_FRAMES_COUNT; ++i) { 
		written[i] = false;
		buffered_av_frames[i] = av_frame_alloc();
		if(buffered_av_frames[i]==NULL) {
			std::cout << "couldnt allocate memory for av frame " << i << "\n";
			return -1;
		}
		avpicture_fill((AVPicture *)buffered_av_frames[i], buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
		buffered_frames[i].init();
	}
	terminated = false;
	int currentFrame = 0;
	
	//start buffering the frames:
	while(!terminated) {
		//std::this_thread::sleep_for(std::chrono::seconds(1));
		if (!written[currentFrame]) {
			//if (get_frames_into_av_buffer(currentFrame)) {
			if (buffer_frame(currentFrame)) {
				++frameswritten;
				//SaveFrame(buffered_av_frames[currentFrame], pCodecCtx->width, pCodecCtx->height, currentFrame*10+7);
				std::cout << "writing frame " << currentFrame << std::endl;
				written[currentFrame] = true;
				++currentFrame;
				currentFrame = currentFrame%BUFFERED_FRAMES_COUNT;
			}
		} else {
			//std::cout << "frame " << currentFrame << " is not read yet" << std::endl;
		}
		//if (currentFrame > 100) {terminated = true;}
		//terminated = true;
		//std::chrono::duration<double> elapsed_seconds = ct2-ct1;
		//std::cout << "elapsed time:" << elapsed_seconds.count() << std::endl;
	}
	std::cout << "coding complete" << std::endl;
}

void initGLUT(int argc, char* argv[]) {
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


void cleanup() {
	// Free the RGB image
	av_free(buffer);
	av_frame_free(&pFrameRGB);
	
	for (int i=0; i<BUFFERED_FRAMES_COUNT; ++i) { 
		av_frame_free(&buffered_av_frames[i]);
	}
  
	// Free the YUV frame
	av_frame_free(&pFrame);
  
	// Close the codecs
	avcodec_close(pCodecCtx);
	avcodec_close(pCodecCtxOrig);

	// Close the video file
	avformat_close_input(&pFormatCtx);
 }

int nextFrame = 0;
int notreadycount = 0;
int winid = 0;

void displayCB(void)		// function called whenever redisplay needed
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

void keyCB(unsigned char key, int x, int y)	/* called on key press */
{
  if( key == 'q' || key == 27 ) glutLeaveMainLoop();
}

int main(int argc, char *argv[]) {
	glutInit(&argc, argv);		// initialize GLUT system
	std::cout << "start!" << std::endl;
	std::thread coder(initFFMPEG, argc, argv);
	//std::this_thread::sleep_for(std::chrono::seconds(1));
	std::thread renderer(initGLUT, argc,argv);

	coder.join();
	renderer.join();
	cleanup();
	std::cout << "written frames: " << frameswritten << std::endl;
	std::cout << "read frames   : " << framesread << std::endl;
	std::cout << "winstuff size: " << win_stuff.size() << std::endl;
	return 0;
}
