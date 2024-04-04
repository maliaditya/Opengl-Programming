// Header Files
#include <windows.h>
#include "OGL.h"

// OpenGL HEADER FILES
#include <GL/glew.h> // This Must Be Before Including <GL/gl.h>
#include <GL/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

#include "Logger.h"


// Standard Header files
#include <stdio.h> // for file io functions
#include <stdlib.h> // for exit()

#define WINWIDTH 800
#define WINHEIGHT 600

// OpenGL  libraries
#pragma comment(lib,"GLEW32.lib")
#pragma comment(lib,"OpenGL32.lib")


// Global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global Variable declarations
BOOL gbActiveWindow = FALSE;
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
BOOL gbFullscreen = FALSE;
FILE* gpFile = NULL;

// programmable pipeline related object
GLuint shaderProgramObject;

enum
{
	ASM_ATTRIBUTE_POSITION = 0,
	ASM_ATTRIBUTE_COLOR,
	ASM_ATTRIBUTE_NORMAL,
	ASM_ATTRIBUTE_TEXTURE0
	
};
GLuint VAO;
GLuint VBO_POSITION;
GLuint VBO_COLOR;
GLuint mvpMatrixUniform;
mat4 perspectiveProjectionMatrix;

Logger logger("CommonError.log");


// Entry Point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{

	// Function declarations
	int initialize(void);
	void display(void);
	void update(void);
	void uninitializer(void);

	// variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyWindow");
	BOOL bDone = FALSE;
	int iRetVal = 0;
	RECT rc;

	// code
	logger.debug("\n Log file is successfully created.\n");

	// initialization of WNDCLASSEX structure
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	// Register WNDCLASSEX 
	RegisterClassEx(&wndclass);

	// Get size of Work Area of the window 
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);

	// create the window
	hwnd = CreateWindowEx(
		WS_EX_APPWINDOW,
		szAppName,
		TEXT("OGL WINDOW"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		(rc.right - WINWIDTH) / 2,
		(rc.bottom - WINHEIGHT) / 2,
		WINWIDTH,
		WINHEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;

	// Initialize
	iRetVal = initialize();

	if (iRetVal == -1)
	{
		logger.debug("Choose Pixel Format Failed\n");
		uninitializer();
	}
	else if (iRetVal == -2)
	{
		logger.debug("Set Pixel Format Failed\n");
		uninitializer();
	}
	else if (iRetVal == -3)
	{
		logger.debug("Create OpenGL Context Failed\n");
		uninitializer();
	}
	else if (iRetVal == -4)
	{
		logger.debug("Making Opengl Context As Current Context Failed\n");
		uninitializer();
	}
	else if (iRetVal == -5)
	{
		logger.debug( "Create OpenGL glewInit Failed \n");
		uninitializer();
	}
	else
	{
		logger.debug("Initialize Success..!");
	}
	

	// show window
	ShowWindow(hwnd, iCmdShow);

	// Foregrounding and focusing the window
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// Game Loop
	while (bDone == FALSE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = TRUE;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow == TRUE)
			{
				// Render the scene
				display();
				
				// Update the scene
				update();

			}
		}
	}


	return((int)msg.wParam);
}

// Callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// Function declarations
	void ToggleFullscreen(void);
	void resize(int, int);
	void uninitializer(void);

	// code
	switch (iMsg)
	{
	

	case WM_SETFOCUS:
		gbActiveWindow = TRUE;
		break;

	case WM_KILLFOCUS:
		gbActiveWindow = FALSE;
		break;

	case WM_ERASEBKGND:
		break;

	case WM_CHAR:
		switch (wParam)
		{
		case 'F':
		case 'f':
			ToggleFullscreen();
			break;

		default:
			break;
		}
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case 27:
			DestroyWindow(hwnd);
			break;

		default:
			break;
		}
		break;

	case WM_SIZE:
		resize(LOWORD(lParam),HIWORD(lParam));
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		uninitializer();
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));

}

void ToggleFullscreen(void)
{
	// Variable declarations
	static DWORD dwStyle;
	static WINDOWPLACEMENT wp;
	MONITORINFO mi;


	// Code
	wp.length = sizeof(WINDOWPLACEMENT);

	if (gbFullscreen == FALSE)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi.cbSize = sizeof(MONITORINFO);
			if (GetWindowPlacement(ghwnd, &wp) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}
			ShowCursor(FALSE);
			gbFullscreen = TRUE;
		}
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wp);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
		gbFullscreen = FALSE;
	}
}

int initialize(void)
{
	// Function declarations
	void printGLInfo(void);
	void uninitializer(void);
	void resize(int width, int height);

	// Variable declarations
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex=0;

	// Code
	// Initialization of PIXELFORMATDESCRIPTOR structure
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;   // 24 can also be done

	// Get DC
	ghdc = GetDC(ghwnd);

	// Choose pixel format
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)
		return(-1);
		
		// set the choosen pixel format
		if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
			return(-2);
				
		// create opengl rendering context
		ghrc = wglCreateContext(ghdc);

		if (ghrc == NULL)
			return(-3);

		//make the rendering context as the current context
		if (wglMakeCurrent(ghdc, ghrc) == FALSE)
			return(-4);

		// Here starts OpenGlCode

		// GLEW Initialization
		if (glewInit() != GLEW_OK)
		{
			return -5;
		}
		// Print OpenGL Info
		printGLInfo();

		// Vertex Shader 
		const GLchar* vertexShaderSourceCode =
			"#version 460 core" \
			"\n" \
			"in vec4 a_position;" \
			"in vec4 a_color;" \
			"uniform mat4 u_mvpMatrix;" \
			"out vec4 a_color_out;" \
			"void main(void)" \
			"{" \
			"a_color_out = a_color;" \
			"gl_Position = u_mvpMatrix * a_position;" \
			"}";
		
		// Creat vertexShaderObject
		GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

		glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);
		
		// compile vertex shader code
		glCompileShader(vertexShaderObject);

		GLint status;
		GLint infoLogLength;
		char* log = NULL;

		glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{
			glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
			if (infoLogLength > 0)
			{
				log = (char*)malloc(infoLogLength);
				if (log != NULL)
				{
					GLsizei written;
					glGetShaderInfoLog(vertexShaderObject, infoLogLength, &written, log);
					logger.debug("Vertex Shader Compilation log:", log);
					free(log);
					uninitializer();

				}
			}
		}

		

		// Fragment Shader
		const GLchar* fragmentShaderSourceCode = 
			"#version 460 core" \
			"\n" \
			"in vec4 a_color_out;" \
			"out vec4 FragColor;" \
			"void main(void)" \
			"{" \
			"FragColor = a_color_out;" \
			"}";

		GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);
		glCompileShader(fragmentShaderObject);

		status = 0;
		infoLogLength=0;
		log = NULL;

		glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{
			glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
			if (infoLogLength > 0)
			{
				log = (char*)malloc(infoLogLength);
				if (log != NULL)
				{
					GLsizei written;
					glGetShaderInfoLog(fragmentShaderObject, infoLogLength, &written, log);
					logger.debug("Fragment Shader Compilation log:", log);
					free(log);
					uninitializer();

				}
			}
		}

		// shader program object
		shaderProgramObject = glCreateProgram();
		
		// attach desired shaders 
		glAttachShader(shaderProgramObject, vertexShaderObject);
		glAttachShader(shaderProgramObject, fragmentShaderObject);

		// Pre-linking binding of shader program object with vertex attributes
		glBindAttribLocation(shaderProgramObject, ASM_ATTRIBUTE_POSITION, "a_position");
		glBindAttribLocation(shaderProgramObject, ASM_ATTRIBUTE_COLOR, "a_color");

		// link the
		glLinkProgram(shaderProgramObject);

		// Posted Linked Retrieving /Getting uniform location  from the shader program object.
		mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_mvpMatrix");

		status = 0;
		infoLogLength = 0;
		log = NULL;


		glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &status);
		if (status == GL_FALSE)
		{
			glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
			if (infoLogLength > 0)
			{
				log = (char*)malloc(infoLogLength);
				if (log != NULL)
				{
					GLsizei written;
					glGetProgramInfoLog(shaderProgramObject, infoLogLength, &written, log);
					logger.debug("Shader Program Link log:", log);
					free(log);
					uninitializer();

				}
			}
		}

		// vao and VBO_POSITION realated code
		//Declare vertex data arrays 
		const GLfloat trianglPosition[] = { 
											0.0f, 1.0f, 0.0f,
										   -1.0f, -1.f, 0.0f,
										    1.0f,-1.0f,0.0f
										};

		const GLfloat trianglColor[] = {
											1.0f,0.0f,0.0f,
											0.0f,1.0f,0.0f,
											0.0f,0.0f,1.0f
									   };

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		// VBO for position
		//Bind with vertex array object and start recording buffer and buffer data related steps.
		glGenBuffers(1, &VBO_POSITION);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_POSITION);
		glBufferData(GL_ARRAY_BUFFER, sizeof(trianglPosition), trianglPosition, GL_STATIC_DRAW);
		glVertexAttribPointer(ASM_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(ASM_ATTRIBUTE_POSITION);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// VBO for Color
		glGenBuffers(1, &VBO_COLOR);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_COLOR);
		glBufferData(GL_ARRAY_BUFFER,sizeof(trianglColor), trianglColor, GL_STATIC_DRAW);
		glVertexAttribPointer(ASM_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(ASM_ATTRIBUTE_COLOR);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);

		// Depth & ClearColor related Changes
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		// Optional
		//glShadeModel(GL_SMOOTH);
		//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	
		
		// clear the screen using black color
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		// Orthographic projection matrix
		perspectiveProjectionMatrix = mat4(1.0);

		// warmup resize
		resize(WINWIDTH, WINHEIGHT);
	return(0);
}

void printGLInfo(void)
{
	// Local Variable declarations
	GLint numExtensions;

	//code
	logger.debug("OpenGL Vendor:", glGetString(GL_VENDOR));
	logger.debug("OpenGL Renderer:", glGetString(GL_RENDERER));
	logger.debug("OpenGL Version:", glGetString(GL_VERSION));
	logger.debug("GLSL Version :", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

	logger.debug("Number Of Supported Extentions: %d \n", numExtensions);

	for (int i = 0; i < numExtensions; i++)
	{
		logger.debug("", glGetStringi(GL_EXTENSIONS,i));

	}

}

void resize(int width, int height)
{
	// code
	if (height == 0)
		height = 1;  //to avoid divide by 0 in future code
	glViewport(0, 0, width, height);

	perspectiveProjectionMatrix = perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void display(void)
{
	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use The Shader Program Object
	glUseProgram(shaderProgramObject);


	// Transformations
	mat4 translationMatrix = mat4(1.0);
	mat4 modelViewMatrix = mat4(1.0);
	mat4 modelViewProjectionMatrix = mat4(1.0);

	modelViewMatrix = translate(modelViewMatrix, vec3(0.0f, 0.0f, -6.0f));
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	
	//Bind with vertex array object
	glBindVertexArray(VAO);

	// Here the should be the Animation/scenes code
	glDrawArrays(GL_TRIANGLES,0,3);

	//Unbind with vertex array object.
	glBindVertexArray(0);

	// unuser the shader program object
	glUseProgram(0);

	SwapBuffers(ghdc);
}

void update(void)
{
	//code

}

void uninitializer(void)
{
	// Function declarations
	void ToggleFullscreen(void);

	// Code
	if (gbFullscreen)
	{
		ToggleFullscreen();
	}
	if (VBO_COLOR)
	{
		glDeleteBuffers(1, &VBO_COLOR);
		VBO_COLOR = 0;
	}

	if (VBO_POSITION)
	{
		glDeleteBuffers(1, &VBO_POSITION);
		VBO_POSITION = 0;
	}

	if (VAO)
	{
		glDeleteVertexArrays(1, &VAO);
		VAO = 0;
	}

	// Shader unintialization
	if (shaderProgramObject)
	{
		
		glUseProgram(shaderProgramObject);

		GLsizei numAttachedShaders;
		glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &numAttachedShaders);
		
		GLuint* shaderObjects = NULL;
		shaderObjects = (GLuint*)malloc(numAttachedShaders * sizeof(GLuint));

		// filling this empty buffer 
		glGetAttachedShaders(shaderProgramObject, numAttachedShaders, &numAttachedShaders, shaderObjects);
		for (GLsizei i = 0; i < numAttachedShaders; i++)
		{
			glDetachShader(shaderProgramObject, shaderObjects[i]);
			glDeleteShader(shaderObjects[i]);
			shaderObjects[i] = 0;
		}

		free(shaderObjects);
		shaderObjects = NULL;
		glUseProgram(0);
		glDeleteProgram(shaderProgramObject);
		shaderProgramObject = 0;
	}

	if (wglGetCurrentContext()==ghrc)
	{
		wglMakeCurrent(NULL, NULL);
	}

	if (ghrc)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}

	if (ghdc)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (ghwnd)
	{
		DestroyWindow(ghwnd);
		ghwnd = NULL;
	}

	
}

