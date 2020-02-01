#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/****************************************************************************
 * DATA STRUCTURES                                                          *
 ****************************************************************************/

#define APP_TITLE "Hello, cube!"

/* Cube: state required for the cube. */
typedef struct {
	GLuint vbo[2];		/* vertex and index buffer names */
	GLuint vao;		/* vertex array object */
	glm::mat4 model;	/* local model transformation */
} Cube;

/* OpenGL debug output error level */
typedef enum {
	DEBUG_OUTPUT_DISABLED=0,
	DEBUG_OUTPUT_ERRORS_ONLY,
	DEBUG_OUTPUT_ALL
} DebugOutputLevel;

/* AppConfig: application configuration, controllable via command line arguments*/
struct AppConfig {
	int posx;
	int posy;
	int width;
	int height;
	bool decorated;
	bool fullscreen;
	unsigned int frameCount;
	DebugOutputLevel debugOutputLevel;
	bool debugOutputSynchronous;

	AppConfig() :
		posx(100),
		posy(100),
		width(800),
		height(600),
		decorated(true),
		fullscreen(false),
		frameCount(0),
		debugOutputLevel(DEBUG_OUTPUT_DISABLED),
		debugOutputSynchronous(false)
	{}
};

/* CubeApp: We encapsulate all of our application state in this struct.
 * We use a single instance of this object (in main), and set a pointer to
 * this as the user-defined pointer for GLFW windows. That way, we have access
 * to this data structure without ever using global variables.
 */
typedef struct {
	/* the window and related state */
	GLFWwindow *win;
	int width, height;
	unsigned int flags;

	/* timing */
	double timeCur, timeDelta;
	double avg_frametime;
	double avg_fps;
	unsigned int frame;

	/* keyboard handling */
	bool pressedKeys[GLFW_KEY_LAST+1];
	bool releasedKeys[GLFW_KEY_LAST+1];

	/* the cube we want to render */
	Cube cube;

	/* the OpenGL state we need for the shaders */
	GLuint program;		/* shader program */
	GLint locProjection;
	GLint locModelView;
	GLint locTime;

	/*  the gloabal transformation matrices */
	glm::mat4 projection;
	glm::mat4 view;
} CubeApp;

/* flags */
#define APP_HAVE_GLFW	0x1	/* we have called glfwInit() and should terminate it */
#define APP_HAVE_GL		0x2	/* we have a valid GL context */

/* We use the following layout for vertex data */
typedef struct {
	GLfloat pos[3]; /* 3D cartesian coordinates */
	GLubyte clr[4]; /* RGBA (8bit per channel is typically enough) */
} Vertex;

/****************************************************************************
 * UTILITY FUNCTIONS: warning output, gl error checking                     *
 ****************************************************************************/

/* Print a info message to stdout, use printf syntax. */
static void info (const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stdout,format, args);
	va_end(args);
	fputc('\n', stdout);
}

/* Print a warning message to stderr, use printf syntax. */
static void warn (const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr,format, args);
	va_end(args);
	fputc('\n', stderr);
}

/* Check for GL errors. If ignore is not set, print a warning if an error was
 * encountered.
 * Returns GL_NO_ERROR if no errors were set. */
static GLenum getGLError(const char *action, bool ignore=false, const char *file=NULL, const int line=0)
{
	GLenum e,err=GL_NO_ERROR;

	do {
		e=glGetError();
		if ( (e != GL_NO_ERROR) && (!ignore) ) {
			err=e;
			if (file)
				fprintf(stderr,"%s:",file);
			if (line)
				fprintf(stderr,"%d:",line);
			warn("GL error 0x%x at %s",(unsigned)err,action);
		}
	} while (e != GL_NO_ERROR);
	return err;
}

/* helper macros:
 * define GL_ERROR_DBG() to be present only in DEBUG builds. This way, you can
 * add error checks at strategic places without influencing the performance
 * of the RELEASE build */
#ifdef NDEBUG
#define GL_ERROR_DBG(action) (void)0
#else
#define GL_ERROR_DBG(action) getGLError(action, false, __FILE__, __LINE__)
#endif

/* define BUFFER_OFFSET to specify offsets inside VBOs */
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

/* define mysnprintf to be either snprintf (POSIX) or sprintf_s (MS Windows) */
#ifdef WIN32
#define mysnprintf sprintf_s
#else
#define mysnprintf snprintf
#endif

/****************************************************************************
 * GL DEBUG MESSAGES                                                        *
 ****************************************************************************/

/* Newer versions of the GL support the generation of human-readable messages
   for GL errors, performance warnings and hints. These messages are
   forwarded to a debug callback which has to be registered with the GL
   context. Debug output may only be available in special debug context... */

/* translate the debug message "source" enum to human-readable string */
static const char *
translateDebugSourceEnum(GLenum source)
{
	const char *s;

	switch (source) {
		case GL_DEBUG_SOURCE_API:
			s="API";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			s="window system";
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			s="shader compiler";
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:
			s="3rd party";
			break;
		case GL_DEBUG_SOURCE_APPLICATION:
			s="application";
			break;
		case GL_DEBUG_SOURCE_OTHER:
			s="other";
			break;
		default:
			s="[UNKNOWN SOURCE]";
	}

	return s;
}

/* translate the debug message "type" enum to human-readable string */
static const char *
translateDebugTypeEnum(GLenum type)
{
	const char *s;

	switch (type) {
		case GL_DEBUG_TYPE_ERROR:
			s="error";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			s="deprecated";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			s="undefined behavior";
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			s="portability";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			s="performance";
			break;
		case GL_DEBUG_TYPE_OTHER:
			s="other";
			break;
		default:
			s="[UNKNOWN TYPE]";
	}
	return s;
}

/* translate the debug message "xeverity" enum to human-readable string */
static const char *
translateDebugSeverityEnum(GLenum severity)
{
	const char *s;

	switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:
			s="high";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			s="medium";
			break;
		case GL_DEBUG_SEVERITY_LOW:
			s="low";
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			s="notification";
			break;
		default:
			s="[UNKNOWN SEVERITY]";
	}

	return s;
}

/* debug callback of the GL */
extern void APIENTRY
debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
			  GLsizei length, const GLchar *message, const GLvoid* userParam)
{
	/* we pass a pointer to our application config to the callback as userParam */
	const AppConfig *cfg=(const AppConfig*)userParam;

	switch(type) {
		case GL_DEBUG_TYPE_ERROR:
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			if (cfg->debugOutputLevel >= DEBUG_OUTPUT_ERRORS_ONLY) {
				warn("GLDEBUG: %s %s %s [0x%x]: %s",
					translateDebugSourceEnum(source),
					translateDebugTypeEnum(type),
					translateDebugSeverityEnum(severity),
					id, message);
			}
			break;
		default:
			if (cfg->debugOutputLevel >= DEBUG_OUTPUT_ALL) {
				warn("GLDEBUG: %s %s %s [0x%x]: %s",
					translateDebugSourceEnum(source),
					translateDebugTypeEnum(type),
					translateDebugSeverityEnum(severity),
					id, message);
			}
	}
}

/****************************************************************************
 * UTILITY FUNCTIONS: print information about the GL context                *
 ****************************************************************************/

/* Print info about the OpenGL context */
static void printGLInfo()
{
	/* get infos about the GL implementation */
	info("OpenGL: %s %s %s",
			glGetString(GL_VENDOR),
			glGetString(GL_RENDERER),
			glGetString(GL_VERSION));
	info("OpenGL Shading language: %s",
			glGetString(GL_SHADING_LANGUAGE_VERSION));
}

/* List all supported GL extensions */
static void listGLExtensions()
{
	GLint num=0;
	GLuint i;
	glGetIntegerv(GL_NUM_EXTENSIONS, &num);
	info("GL extensions supported: %d", num);
	if (num < 1) {
		return;
	}

	for (i=0; i<(GLuint)num; i++) {
		const GLubyte *ext=glGetStringi(GL_EXTENSIONS,i);
		if (ext) {
			info("  %s",ext);
		}
	}
}

/****************************************************************************
 * SETTING UP THE GL STATE                                                  *
 ****************************************************************************/

/* Initialize the global OpenGL state. This is called once after the context
 * is created. */
static void initGLState(const AppConfig&cfg)
{
	printGLInfo();
	listGLExtensions();

	if (cfg.debugOutputLevel > DEBUG_OUTPUT_DISABLED) {
		if (GLAD_GL_VERSION_4_3) {
			info("enabling GL debug output [via OpenGL >= 4.3]");
			glDebugMessageCallback(debugCallback,&cfg);
			glEnable(GL_DEBUG_OUTPUT);
			if (cfg.debugOutputSynchronous) {
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			} else {
				glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			}
		} else if (GLAD_GL_KHR_debug) {
			info("enabling GL debug output [via GL_KHR_debug]");
			glDebugMessageCallback(debugCallback,&cfg);
			glEnable(GL_DEBUG_OUTPUT);
			if (cfg.debugOutputSynchronous) {
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			} else {
				glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			}
		} else if (GLAD_GL_ARB_debug_output) {
			info("enabling GL debug output [via GL_ARB_debug_output]");
			glDebugMessageCallbackARB(debugCallback,&cfg);
			if (cfg.debugOutputSynchronous) {
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
			} else {
				glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
			}
		} else {
			warn("GL debug output requested, but not supported by the context");
		}
	}

	/* we set these once and never change them, so there is no need
	 * to set them during the main loop */
	glEnable(GL_DEPTH_TEST);

	/* We do not enable backface culling, since the "cut" shader works
	 * best when one can see through the cut-out front faces... */
	//glEnable(GL_CULL_FACE);
}

/****************************************************************************
 * SHADER COMPILATION AND LINKING                                           *
 ****************************************************************************/

/* Print the info log of the shader compiler/linker.
 * If program is true, obj is assumed to be a program object, otherwise, it
 * is assumed to be a shader object.
 */
static void printInfoLog(GLuint obj, bool program)
{
	char log[16384];
	if (program) {
		glGetProgramInfoLog(obj, sizeof(log), 0, log);
	} else {
		glGetShaderInfoLog(obj, sizeof(log), 0, log);
	}
	/* technically, this is not strictly necessary as the GL implementation
	 * is required to properly terminate the string, but we never trust
	 * other code and make sure the string is terminated before running out
	 * of the buffer. */
	log[sizeof(log)-1]=0;
	fprintf(stderr,"%s\n",log);
}

/* Create a new shader object, attach "source" as source string,
 * and compile it.
 * Returns the name of the newly created shader object, or 0 in case of an
 * error.
 */
static  GLuint shaderCreateAndCompile(GLenum type, const GLchar *source)
{
	GLuint shader=0;
	GLint status;

	shader=glCreateShader(type);
	info("created shader object %u",shader);
	glShaderSource(shader, 1, (const GLchar**)&source, NULL);
	info("compiling shader object %u",shader);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		warn("Failed to compile shader");
		printInfoLog(shader,false);
		glDeleteShader(shader);
		shader=0;
	}

	return shader;
}

/* Create a new shader object by loading a file, and compile it.
 * Returns the name of the newly created shader object, or 0 in case of an
 * error.
 */
static  GLuint shaderCreateFromFileAndCompile(GLenum type, const char *filename)
{

	info("loading shader file '%s'",filename);
	FILE *file = fopen(filename, "rt");
	if(!file) {
		warn("Failed to open shader file '%s'", filename);
		return 0;
	}
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	GLchar *source = (GLchar*)malloc(size+1);
	if (!source) {
		warn("Failed to allocate memory for shader file '%s'", filename);
		fclose(file);
		return 0;
	}
	fseek(file, 0, SEEK_SET);
	source[fread(source, 1, size, file)] = 0;
	fclose(file);

	GLuint shader=shaderCreateAndCompile(type, source);
	free(source);
	return shader;
}

/* Create a program by linking a vertex and fragment shader object. The shader
 * objects should already be compiled.
 * Returns the name of the newly created program object, or 0 in case of an
 * error.
 */
static GLuint programCreate(GLuint vertex_shader, GLuint fragment_shader)
{
	GLuint program=0;
	GLint status;

	program=glCreateProgram();
	info("created program %u",program);

	if (vertex_shader)
		glAttachShader(program, vertex_shader);
	if (fragment_shader)
		glAttachShader(program, fragment_shader);

	/* hard-code the attribute indices for the attributeds we use */
	glBindAttribLocation(program, 0, "pos");
	glBindAttribLocation(program, 1, "nrm");
	glBindAttribLocation(program, 2, "clr");
	glBindAttribLocation(program, 3, "tex");

	/* hard-code the color number of the fragment shader output */
	glBindFragDataLocation(program, 0, "color");

	/* finally link the program */
	info("linking program %u",program);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		warn("Failed to link program!");
		printInfoLog(program,true);
		glDeleteProgram(program);
		return 0;
	}
	return program;
}

/* Create a program object directly from vertex and fragment shader source
 * files.
 * Returns the name of the newly created program object, or 0 in case of an
 * error.
 */
static GLenum programCreateFromFiles(const char *vs, const char *fs)
{
	GLuint id_vs=shaderCreateFromFileAndCompile(GL_VERTEX_SHADER, vs);
	GLuint id_fs=shaderCreateFromFileAndCompile(GL_FRAGMENT_SHADER, fs);
	GLuint program=programCreate(id_vs,id_fs);
	/* Delete the shader objects. Since they are still in use in the
	 * program object, OpenGL will not destroy them internally until
	 * the program object is destroyed. The caller of this function
	 * does not need to care about the shader objects at all. */
	info("destroying shader object %u",id_vs);
	glDeleteShader(id_vs);
	info("destroying shader object %u",id_fs);
	glDeleteShader(id_fs);
	return program;
}
/****************************************************************************
 * THE SHADERS WE USE                                                       *
 ****************************************************************************/

/* In this example, we load the shaders from file, and are able to re-load
 * them on keypress. */

/* Destroy all GL objects related to the shaders. */
static void destroyShaders(CubeApp *app)
{
	if (app->program) {
		info("deleting program %u",app->program);
		glDeleteProgram(app->program);
		app->program=0;
	}
}

/* Create and compile the shaders and link them to a program and qeury
 * the locations of the uniforms we use.
 * Returns true if successfull and false in case of an error. */
static bool initShaders(CubeApp *app, const char *vs, const char *fs)
{
	destroyShaders(app);
	app->program=programCreateFromFiles(vs, fs);
	if (app->program == 0)
		return false;

	app->locProjection=glGetUniformLocation(app->program, "projection");
	app->locModelView=glGetUniformLocation(app->program, "modelView");
	app->locTime=glGetUniformLocation(app->program, "time");
	info("program %u: location for \"projection\" uniform: %d",app->program, app->locProjection);
	info("program %u: location for \"modelView\" uniform: %d",app->program, app->locModelView);
	info("program %u: location for \"time\" uniform: %d",app->program, app->locTime);

	return true;
}

/****************************************************************************
 * THE CUBE...                                                              *
 ****************************************************************************/

/* Initialize the OpenGL state for the cube. This will create OpenGL buffer
 * objects for storing the vertex and index arrays and the OpenGL Vertex
 * Array. The buffers will be filled with the data, and the VAO will be
 * initialized so that the vertex array layout and offsets in the buffer
 * will be set.
 *
 * This function is only called once. After it returned, all the data needed
 * for drawing the cube is stored in GL objects, so we do not have to
 * re-specify the vertex data every time the object is drawn. */
static void initCube(Cube *cube)
{
	static const Vertex cubeGeometry[]={
		/*   X     Y     Z       R    G    B    A */
		/* front face */
		{{-1.0, -1.0,  1.0},  {255,   0,   0, 255}},
		{{ 1.0, -1.0,  1.0},  {192,   0,   0, 255}},
		{{-1.0,  1.0,  1.0},  {192,   0,   0, 255}},
		{{ 1.0,  1.0,  1.0},  {128,   0,   0, 255}},
		/* back face */
		{{ 1.0, -1.0, -1.0},  {  0, 255, 255, 255}},
		{{-1.0, -1.0, -1.0},  {  0, 192, 192, 255}},
		{{ 1.0,  1.0, -1.0},  {  0, 192, 192, 255}},
		{{-1.0,  1.0, -1.0},  {  0, 128, 128, 255}},
		/* left  face */
		{{-1.0, -1.0, -1.0},  {  0, 255,   0, 255}},
		{{-1.0, -1.0,  1.0},  {  0, 192,   0, 255}},
		{{-1.0,  1.0, -1.0},  {  0, 192,   0, 255}},
		{{-1.0,  1.0,  1.0},  {  0, 128,   0, 255}},
		/* right face */
		{{ 1.0, -1.0,  1.0},  {255,   0, 255, 255}},
		{{ 1.0, -1.0, -1.0},  {192,   0, 192, 255}},
		{{ 1.0,  1.0,  1.0},  {192,   0, 192, 255}},
		{{ 1.0,  1.0, -1.0},  {128,   0, 128, 255}},
		/* top face */
		{{-1.0,  1.0,  1.0},  {  0,   0, 255, 255}},
		{{ 1.0,  1.0,  1.0},  {  0,   0, 192, 255}},
		{{-1.0,  1.0, -1.0},  {  0,   0, 192, 255}},
		{{ 1.0,  1.0, -1.0},  {  0,   0, 128, 255}},
		/* bottom face */
		{{ 1.0, -1.0,  1.0},  {255, 255,   0, 255}},
		{{-1.0, -1.0,  1.0},  {192, 192,   0, 255}},
		{{ 1.0, -1.0, -1.0},  {192, 192,   0, 255}},
		{{-1.0, -1.0, -1.0},  {128, 128,   0, 255}},
	};

	/* use two triangles sharing an edge for each face */
	static const GLushort cubeConnectivity[]={
		 0, 1, 2,  2, 1, 3,	/* front */
		 4, 5, 6,  6, 5, 7,	/* back */
		 8, 9,10, 10, 9,11,	/* left */
		12,13,14, 14,13,15,	/* right */
		16,17,18, 18,17,19,	/* top */
		20,21,22, 22,21,23	/* bottom */
	};

	/* set up VAO and vertex and element array buffers */
	glGenVertexArrays(1,&cube->vao);
	glBindVertexArray(cube->vao);
	info("Cube: created VAO %u", cube->vao);

	glGenBuffers(2,cube->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, cube->vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeGeometry), cubeGeometry, GL_STATIC_DRAW);
	info("Cube: created VBO %u for %u bytes of vertex data", cube->vbo[0], (unsigned)sizeof(cubeGeometry));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube->vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeConnectivity), cubeConnectivity, GL_STATIC_DRAW);
	info("Cube: created VBO %u for %u bytes of element data", cube->vbo[1], (unsigned)sizeof(cubeConnectivity));

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex,pos)));
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex,clr)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	cube->model = glm::mat4(1.0f);
	GL_ERROR_DBG("cube initialization");
}

/* Destroy all GL objects related to the cube. */
static void destroyCube(Cube *cube)
{
	glBindVertexArray(0);
	if (cube->vao) {
		info("Cube: deleting VAO %u", cube->vao);
		glDeleteVertexArrays(1,&cube->vao);
		cube->vao=0;
	}
	if (cube->vbo[0] || cube->vbo[1]) {
		info("Cube: deleting VBOs %u %u", cube->vbo[0], cube->vbo[1]);
		glDeleteBuffers(2,cube->vbo);
		cube->vbo[0]=0;
		cube->vbo[1]=0;
	}
}

/****************************************************************************
 * WINDOW-RELATED CALLBACKS                                                 *
 ****************************************************************************/

/* This function is registered as the framebuffer size callback for GLFW,
 * so GLFW will call this whenever the window is resized. */
static void callback_Resize(GLFWwindow *win, int w, int h)
{
	CubeApp *app=(CubeApp*)glfwGetWindowUserPointer(win);
	info("new framebuffer size: %dx%d pixels",w,h);

	/* store curent size for later use in the main loop */
	app->width=w;
	app->height=h;

	/* we _could_ directly set the viewport here ... */
}

/* This function is registered as the keayboard callback for GLFW, so GLFW
 * will call this whenever a key is pressed. */
static void callback_Keyboard(GLFWwindow *win, int key, int scancode, int action, int mods)
{
	/* The shaders we load on the number keys. We always load a combination of
	 * a vertex and a fragment shader. */
	static const char* shaders[][2]={
		/* 0 */ {"shaders/minimal.vs.glsl", "shaders/minimal.fs.glsl"},
		/* 1 */ {"shaders/color.vs.glsl", "shaders/color.fs.glsl"},
		/* 2 */ {"shaders/cut.vs.glsl", "shaders/cut.fs.glsl"},
		/* 3 */ {"shaders/wobble.vs.glsl", "shaders/color.fs.glsl"},
		/* 4 */ {"shaders/experimental.vs.glsl", "shaders/experimental.fs.glsl"},
		/* placeholders for additional shaders */
		/* 5 */ {"shaders/yourshader.vs.glsl", "shaders/yourshader.fs.glsl"},
		/* 6 */ {"shaders/yourshader.vs.glsl", "shaders/yourshader.fs.glsl"},
		/* 7 */ {"shaders/yourshader.vs.glsl", "shaders/yourshader.fs.glsl"},
		/* 8 */ {"shaders/yourshader.vs.glsl", "shaders/yourshader.fs.glsl"},
		/* 9 */ {"shaders/yourshader.vs.glsl", "shaders/yourshader.fs.glsl"}
	};

	CubeApp *app=(CubeApp*)glfwGetWindowUserPointer(win);

	if (key < 0 || key > GLFW_KEY_LAST) {
		warn("invalid key code %d?!",key);
		return;
	}

	if (action == GLFW_RELEASE) {
		app->pressedKeys[key] = false;
	} else {
		if (!app->pressedKeys[key]) {
			/* handle certain keys */
			if (key >= '0' && key <= '9') {
				initShaders(app, shaders[key - '0'][0], shaders[key - '0'][1]);
			} else {
				switch (key) {
					case GLFW_KEY_ESCAPE:
						glfwSetWindowShouldClose(win,1);
						break;
				}
			}
		}
		app->pressedKeys[key] = true;
	}
}

/****************************************************************************
 * GLOBAL INITIALIZATION AND CLEANUP                                        *
 ****************************************************************************/

/* Initialize the Cube Application.
 * This will initialize the app object, create a windows and OpenGL context
 * (via GLFW), initialize the GL function pointers via GLEW and initialize
 * the cube.
 * Returns true if successfull or false if an error occured. */
bool initCubeApplication(CubeApp *app, const AppConfig& cfg)
{
	int i;
	int w, h, x, y;
	bool debugCtx=(cfg.debugOutputLevel > DEBUG_OUTPUT_DISABLED);

	/* Initialize the app structure */
	app->win=NULL;
	app->flags=0;
	app->avg_frametime=-1.0;
	app->avg_fps=-1.0;
	app->frame = 0;

	for (i=0; i<=GLFW_KEY_LAST; i++)
		app->pressedKeys[i]=app->releasedKeys[i]=false;

	app->cube.vbo[0]=app->cube.vbo[1]=app->cube.vao=0;
	app->program=0;

	/* initialize GLFW library */
	info("initializing GLFW");
	if (!glfwInit()) {
		warn("Failed to initialze GLFW");
		return false;
	}

	app->flags |= APP_HAVE_GLFW;

	/* request a OpenGL 3.2 core profile context */
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, (debugCtx)?GL_TRUE:GL_FALSE);

	GLFWmonitor *monitor = NULL;
	x = cfg.posx;
	y = cfg.posy;
	w = cfg.width;
	h = cfg.height;

	if (cfg.fullscreen) {
		monitor = glfwGetPrimaryMonitor();
	}
	if (monitor) {
		glfwGetMonitorPos(monitor, &x, &y);
		const GLFWvidmode *v = glfwGetVideoMode(monitor);
		if (v) {
			w = v->width;
			h = v->height;
			info("Primary monitor: %dx%d @(%d,%d)", w, h, x, y);
		}
		else {
			warn("Failed to query current video mode!");
		}
	}

	if (!cfg.decorated) {
		glfwWindowHint(GLFW_DECORATED, GL_FALSE);
	}

	/* create the window and the gl context */
	info("creating window and OpenGL context");
	app->win=glfwCreateWindow(w, h, APP_TITLE, monitor, NULL);
	if (!app->win) {
		warn("failed to get window with OpenGL 3.2 core context");
		return false;
	}

	app->width = w;
	app->height = h;

	if (!monitor) {
		glfwSetWindowPos(app->win, x, y);
	}

	/* store a pointer to our application context in GLFW's window data.
	 * This allows us to access our data from within the callbacks */
	glfwSetWindowUserPointer(app->win, app);
	/* register our callbacks */
	glfwSetFramebufferSizeCallback(app->win, callback_Resize);
	glfwSetKeyCallback(app->win, callback_Keyboard);

	/* make the context the current context (of the current thread) */
	glfwMakeContextCurrent(app->win);

	/* ask the driver to enable synchronizing the buffer swaps to the
	 * VBLANK of the display. Depending on the driver and the user's
	 * setting, this may have no effect. But we can try... */
	glfwSwapInterval(1);

	/* initialize glad,
	 * this will load all OpenGL function pointers
	 */
	info("initializing glad");
	if (!gladLoadGL(glfwGetProcAddress)) {
		warn("failed to intialize glad GL extension loader");
		return false;
	}

	if (!GLAD_GL_VERSION_3_2) {
		warn("failed to load at least GL 3.2 functions via GLAD");
		return false;
	}

	app->flags |= APP_HAVE_GL;

	/* initialize the GL context */
	initGLState(cfg);
	initCube(&app->cube);
	if (!initShaders(app,"shaders/color.vs.glsl","shaders/color.fs.glsl")) {
		warn("something wrong with our shaders...");
		return false;
	}

	/* initialize the timer */
	app->timeCur=glfwGetTime();

	return true;
}

/* Clean up: destroy everything the cube app still holds */
static void destroyCubeApp(CubeApp *app)
{
	if (app->flags & APP_HAVE_GLFW) {
		if (app->win) {
			if (app->flags & APP_HAVE_GL) {
				destroyCube(&app->cube);
				destroyShaders(app);
			}
			glfwDestroyWindow(app->win);
		}
		glfwTerminate();
	}
}

/****************************************************************************
 * DRAWING FUNCTION                                                         *
 ****************************************************************************/

/* This draws the complete scene for a single eye */
static void
drawScene(CubeApp *app)
{
	/* combine model and view matrices to the modelView matrix our
	 * shader expects */
	glm::mat4 modelView = app->view * app->cube.model;

	/* use the program and update the uniforms */
	glUseProgram(app->program);
	glUniformMatrix4fv(app->locProjection, 1, GL_FALSE, glm::value_ptr(app->projection));
	glUniformMatrix4fv(app->locModelView, 1, GL_FALSE, glm::value_ptr(modelView));
	glUniform1f(app->locTime, (GLfloat)app->timeCur);

	/* draw the cube */
	glBindVertexArray(app->cube.vao);
	glDrawElements(GL_TRIANGLES, 6 * 6, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

	/* "unbind" the VAO and the program. We do not have to do this.
	* OpenGL is a state machine. The last binings will stay effective
	* until we actively change them by binding something else. */
	glBindVertexArray(0);
}

/* Set up projection and view matrices
 * Although this is constant in this example, we re-calculate it per frame.
 * For a moving camera, the view matrix would typically change per frame.
 */
static void
setProjectionAndView(CubeApp *app)
{
	app->projection = glm::perspective(glm::radians(75.0f), (float)app->width / (float)app->height, 0.1f, 10.0f);
	app->view = glm::translate(glm::vec3(0.0f, 0.0f, -4.0f));
}

/* The main drawing function. This is responsible for drawing the next frame,
 * it is called in a loop as long as the application runs */
static void
displayFunc(CubeApp *app, const AppConfig& cfg)
{
	/* rotate the cube */
	app->cube.model = glm::rotate(app->cube.model, (float)(glm::half_pi<double>() * app->timeDelta), glm::vec3(0.8f, 0.6f, 0.1f));

	/* set the viewport (might have changed since last iteration) */
	glViewport(0, 0, app->width, app->height);

	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); /* clear the buffers */

	setProjectionAndView(app);
	drawScene(app);

	/* finished with drawing, swap FRONT and BACK buffers to show what we
	 * have rendered */
	glfwSwapBuffers(app->win);

	/* In DEBUG builds, we also check for GL errors in the display
	 * function, to make sure no GL error goes unnoticed. */
	GL_ERROR_DBG("display function");
}

/****************************************************************************
 * MAIN LOOP                                                                *
 ****************************************************************************/

/* The main loop of the application. This will call the display function
 *  until the application is closed. This function also keeps timing
 *  statistics. */
static void mainLoop(CubeApp *app, const AppConfig& cfg)
{
	unsigned int frame=0;
	double start_time=glfwGetTime();
	double last_time=start_time;

	info("entering main loop");
	while (!glfwWindowShouldClose(app->win)) {
		/* update the current time and time delta to last frame */
		double now=glfwGetTime();
		app->timeDelta = now - app->timeCur;
		app->timeCur = now;

		/* update FPS estimate at most once every second */
		double elapsed = app->timeCur - last_time;
		if (elapsed >= 1.0) {
			char WinTitle[80];
			app->avg_frametime=1000.0 * elapsed/(double)frame;
			app->avg_fps=(double)frame/elapsed;
			last_time=app->timeCur;
			frame=0;
			/* update window title */
			mysnprintf(WinTitle, sizeof(WinTitle), APP_TITLE "   /// AVG: %4.2fms/frame (%.1ffps)", app->avg_frametime, app->avg_fps);
			glfwSetWindowTitle(app->win, WinTitle);
			info("frame time: %4.2fms/frame (%.1ffps)",app->avg_frametime, app->avg_fps);
		}

		/* call the display function */
		displayFunc(app, cfg);
		app->frame++;
		frame++;
		if (cfg.frameCount && app->frame >= cfg.frameCount) {
			break;
		}
		/* This is needed for GLFW event handling. This function
		 * will call the registered callback functions to forward
		 * the events to us. */
		glfwPollEvents();
	}
	info("left main loop\n%u frames rendered in %.1fs seconds == %.1ffps",
		app->frame,(app->timeCur-start_time),
		(double)app->frame/(app->timeCur-start_time) );
}

/****************************************************************************
 * SIMPLE COMMAND LINE PARSER                                               *
 ****************************************************************************/

void parseCommandlineArgs(AppConfig& cfg, int argc, char**argv)
{
	for (int i = 1; i < argc; i++) {
		if (!std::strcmp(argv[i], "--fullscreen")) {
			cfg.fullscreen = true;
			cfg.decorated = false;
		} else if (!std::strcmp(argv[i], "--undecorated")) {
			cfg.decorated = false;
		} else if (!std::strcmp(argv[i], "--gl-debug-sync")) {
			cfg.debugOutputSynchronous = true;
		}
		else if (i + 1 < argc) {
			if (!std::strcmp(argv[i], "--width")) {
				cfg.width = (int)strtol(argv[++i], NULL, 10);
			} else if (!std::strcmp(argv[i], "--height")) {
				cfg.height = (int)strtol(argv[++i], NULL, 10);
			} else if (!std::strcmp(argv[i], "--x")) {
				cfg.posx = (int)strtol(argv[++i], NULL, 10);
			} else if (!std::strcmp(argv[i], "--y")) {
				cfg.posy = (int)strtol(argv[++i], NULL, 10);
			} else if (!std::strcmp(argv[i], "--frameCount")) {
				cfg.frameCount = (unsigned)strtoul(argv[++i], NULL, 10);
			} else if (!std::strcmp(argv[i], "--gl-debug-level")) {
				cfg.debugOutputLevel = (DebugOutputLevel)strtoul(argv[++i], NULL, 10);
			}
		}
	}
}

/****************************************************************************
 * PROGRAM ENTRY POINT                                                      *
 ****************************************************************************/

int main (int argc, char **argv)
{
	AppConfig cfg;	/* the generic configuration */
	CubeApp app;	/* the cube application stata stucture */

	parseCommandlineArgs(cfg, argc, argv);

	if (initCubeApplication(&app, cfg)) {
		/* initialization succeeded, enter the main loop */
		mainLoop(&app, cfg);
	}
	/* clean everything up */
	destroyCubeApp(&app);

	return 0;
}

