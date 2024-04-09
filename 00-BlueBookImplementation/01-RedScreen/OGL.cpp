// Header Files
#include <windows.h>
#include "OGL.h"

// OpenGL HEADER FILES
#include <GL/glew.h> // This Must Be Before Including <GL/gl.h>
#include <GL/gl.h>


// Standard Header files
#include <stdio.h> // for file io functions
#include <stdlib.h> // for exit()
#include <cmath>

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


// Entry Point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// Function declarations
	int initialize(void);
	void display(void);
	void update(void);
	void uninitialize(void);

	// variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyWindow");
	BOOL bDone = FALSE;
	int iRetVal = 0;
	RECT rc;

	// code
	if (fopen_s(&gpFile, "Log.txt", "w") != 0) // fopen_s secure/save r/w/a(append) or r+/w+/a+
	{
		MessageBox(NULL, TEXT("Creation of log file failed. Exiting..."), TEXT("File I/O Error"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log file is successfully created.\n");
	}

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
		fprintf(gpFile, "Choose Pixel Format Failed\n");
		uninitialize();
	}
	else if (iRetVal == -2)
	{
		fprintf(gpFile, "Set Pixel Format Failed\n");
		uninitialize();
	}
	else if (iRetVal == -3)
	{
		fprintf(gpFile, "Create OpenGL Context Failed\n");
		uninitialize();
	}
	else if (iRetVal == -4)
	{
		fprintf(gpFile, "Making Opengl Context As Current Context Failed\n");
		uninitialize();
	}
	else if (iRetVal == -5)
	{
		fprintf(gpFile, "Create OpenGL glewInit Failed \n");
		uninitialize();
	}
	else
	{
		fprintf(gpFile, "Initialize Success..!\n");
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
	void uninitialize(void);

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
		uninitialize();
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
	void uninitialize(void);
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
			"void main(void)" \
			"{" \
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
					fprintf(gpFile, "Vertex Shader Compilation log: %s \n", log);
					free(log);
					uninitialize();

				}
			}
		}

		

		// Fragment Shader
		const GLchar* fragmentShaderSourceCode = 
			"#version 460 core" \
			"\n" \
			"void main(void)" \
			"{" \
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
					fprintf(gpFile, "Fragment Shader Compilation log: %s \n", log);
					free(log);
					uninitialize();

				}
			}
		}

		// shader program object
		shaderProgramObject = glCreateProgram();
		
		// attach desired shaders 
		glAttachShader(shaderProgramObject, vertexShaderObject);
		glAttachShader(shaderProgramObject, fragmentShaderObject);

		// link the
		glLinkProgram(shaderProgramObject);


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
					fprintf(gpFile, "Shader Program Link log: %s \n", log);
					free(log);
					uninitialize();

				}
			}
		}


		
		 
		// Depth related Changes
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		// Optional
		//glShadeModel(GL_SMOOTH);
		//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		// clear the screen using blue color
		glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

	return(0);
}

void printGLInfo(void)
{
	// Local Variable declarations
	GLint numExtensions;

	//code
	fprintf(gpFile, "OpenGL Vendor: %s \n", glGetString(GL_VENDOR));
	fprintf(gpFile, "OpenGL Renderer: %s \n", glGetString(GL_RENDERER));
	fprintf(gpFile, "OpenGL Version: %s \n", glGetString(GL_VERSION));
	fprintf(gpFile, "GLSL Version : %s \n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

	fprintf(gpFile, "Number Of Supported Extentions: %d \n", numExtensions);

	for (int i = 0; i < numExtensions; i++)
	{
		fprintf(gpFile, " %s \n", glGetStringi(GL_EXTENSIONS,i));

	}

}

void resize(int width, int height)
{
	// code
	if (height == 0)
		height = 1;  //to avoid divide by 0 in future code
	glViewport(0, 0, width, height);
}

float deltaTime = 0.0f;
float lastFrame = 0.0f;

double getTime() {
	LARGE_INTEGER frequency;
	LARGE_INTEGER currentTime;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&currentTime);

	return (double)currentTime.QuadPart / (double)frequency.QuadPart;
}


void display(void)
{
	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// float currentFrame = getTime();
	// deltaTime = currentFrame - lastFrame;
	// lastFrame = currentFrame;

	const GLfloat color[] = {
		(float)sin(getTime()) * 0.5f + 0.5f,
		(float)sin(getTime()) * 0.5f + 0.5f,
		0.0f,1.0f
	};

	glClearBufferfv(GL_COLOR, 0, color);

	// Use The Shader Program Object
	glUseProgram(shaderProgramObject);

	// Here the should be the Animation/scenes code


	// unuser the shader program object
	glUseProgram(0);

	SwapBuffers(ghdc);
}

void update(void)
{
	//code

}

void uninitialize(void)
{
	// Function declarations
	void ToggleFullscreen(void);

	// Code
	if (gbFullscreen)
	{
		ToggleFullscreen();
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

	if (gpFile)
	{
		fprintf(gpFile, "Log file is successfuly closed.\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}

