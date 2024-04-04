#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_opengl3.h"
#include "Imgui/imgui_impl_win32.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include <GL/glew.h> // This Must Be Before Including <GL/gl.h>
#include <GL/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <tchar.h>

// Standard Header files
#include <stdio.h> // for file io functions
#include <stdlib.h> // for exit()

#include <map>
#include <vector>
using namespace glm;
using namespace std;
typedef BOOL(APIENTRY* PFNWGLSWAPINTERVALEXTPROC)(int interval);
// Standard Header files
//#include <stdio.h> // for file io functions
//#include <stdlib.h> // for exit()

// OpenGL  libraries
#pragma comment(lib,"GLEW32.lib")
#pragma comment(lib,"OpenGL32.lib")

#define WINWIDTH 800
#define WINHEIGHT 600

#include "Animator.h"
// Assuming Logger.h contains the definition of your logger class

int animation = 0;
int nextAnimation = 0;
int checkAnimation = 0;
int checkNextAnimation = 0;
bool startRotation = false;
GLfloat translatex=0.0f;
GLfloat translatey=-1.0f;
GLfloat translatez = -4.0f;

std::vector<std::string> animationNames;
GLfloat animationBlendFactor = 0.0f;
void debugTransforms(const std::vector<glm::mat4>& transforms) {
	Logger logger("debugTransforms.log");
	std::stringstream ss;

	for (int i = 0; i < transforms.size(); ++i) {
		glm::mat4 transform = transforms[i];
		ss << "Transform " << i << ":" << std::endl;
		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				ss << transform[row][col] << " ";
			}
			ss << std::endl;
		}
	}

	logger.debug("debugTransforms", ss.str());
}
// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
// Global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global Variable declarations
BOOL gbActiveWindow = FALSE;
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
BOOL gbFullscreen = FALSE;
FILE* gpFile = NULL;
GLfloat angleCube = 0.0f;
GLfloat scalevalue = 0.01f;
static int              g_Width;
static int              g_Height;
Model mdl;

string model = "Model/iclone_7_raptoid_mascot_-_free_download.glb";
//string model = "D:/Aadis GFX LAB/Models/Flair (1).fbx";

// programmable pipeline related object
GLuint shaderProgramObject;

float angle = 1.0f;
// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

double getTime() {
	LARGE_INTEGER frequency;
	LARGE_INTEGER currentTime;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&currentTime);

	return (double)currentTime.QuadPart / (double)frequency.QuadPart;
}

Animator animator;
Animation danceAnimation;
Animation danceAnimation2;
// Function to populate the vertices vector with square positions
Mesh PopulateSquareVertices(const GLfloat* squarePosition, int numVertices) {
	vector<Vertex> verticesa;
	// Clear the vertices vector to start fresh
	// Populate the vertices with square positions
	for (int i = 0; i < numVertices; i += 3) {
		Vertex vertex;
		vertex.Position = glm::vec3(squarePosition[i], squarePosition[i + 1], squarePosition[i + 2]);
		// You can set other vertex attributes such as Normal, TexCoords, etc. if needed
		verticesa.push_back(vertex);
	}
	return Mesh(verticesa);
}



// Entry Point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// Function declarations
	int initialize(void);
	void display(void);
	void update(void);
	void myuninitialize(void);

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
		TEXT("FALCON Engine"),
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
		myuninitialize();
	}
	else if (iRetVal == -2)
	{
		fprintf(gpFile, "Set Pixel Format Failed\n");
		myuninitialize();
	}
	else if (iRetVal == -3)
	{
		fprintf(gpFile, "Create OpenGL Context Failed\n");
		myuninitialize();
	}
	else if (iRetVal == -4)
	{
		fprintf(gpFile, "Making Opengl Context As Current Context Failed\n");
		myuninitialize();
	}
	else if (iRetVal == -5)
	{
		fprintf(gpFile, "Create OpenGL glewInit Failed \n");
		myuninitialize();
	}
	else
	{
		fprintf(gpFile, "Initialize Success..!\n");
	}


	// show window
	ShowWindow(hwnd, iCmdShow);




	// Initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad | ImGuiConfigFlags_DockingEnable;
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(ghwnd);
	ImGui_ImplOpenGL3_Init("#version 130");



	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


	// Foregrounding and focusing the window
	//SetForegroundWindow(hwnd);
	//SetFocus(hwnd);

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


	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	return((int)msg.wParam);
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWhwnnd, UINT msg, WPARAM wParam, LPARAM lParam);


// Callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// Function declarations
	void ToggleFullscreen(void);
	void resize(int, int);
	void myuninitialize(void);

	ImGui_ImplWin32_WndProcHandler(hwnd, iMsg, wParam, lParam);

	// code
	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbActiveWindow = TRUE;
		break;

	case WM_KILLFOCUS:
		gbActiveWindow = FALSE;
		break;

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
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

		case 'S':
			scalevalue = scalevalue - 0.1f;
			break;
		case 's':
			scalevalue = scalevalue + 0.1f;
			break;

		case 'R':
			angleCube = angleCube + 0.1f;
			break;

		case 'r':
			angleCube = angleCube - 0.1f;

			break;

		case 'q':
			animation = animation + 1;
			danceAnimation = Animation(model, &mdl, animation);
			animator = Animator(&danceAnimation);
			break;

		case 'Q':
			animation = animation - 1;
			danceAnimation = Animation(model, &mdl, animation);
			animator = Animator(&danceAnimation);
			break;
		case 'w':
			animation = 0;
			danceAnimation = Animation(model, &mdl, animation);
			animator = Animator(&danceAnimation);
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
		resize(LOWORD(lParam), HIWORD(lParam));
		if (wParam != SIZE_MINIMIZED)
		{
			g_Width = LOWORD(lParam);
			g_Height = HIWORD(lParam);
		}
		return 0;
		break;
	

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		myuninitialize();
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
	void myuninitialize(void);
	void resize(int width, int height);

	// Variable declarations
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;

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
	glewExperimental = GL_TRUE;
	// GLEW Initialization
	if (glewInit() != GLEW_OK)
	{
		return -5;
	}

	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = nullptr;
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");


	if (wglSwapIntervalEXT != nullptr) {
		// Call the function to enable VSync
		wglSwapIntervalEXT(1); // 1 for enabled, 0 for disabled
	}

	// Vertex Shader 
	const GLchar* vertexShaderSourceCode =
		"#version 460 core" \
		"\n" \
		"in vec4 a_position;" \
		"in vec2 a_texcoord;" \
		"in ivec4 boneIds;" \
		"in vec4 weights;" \
		"const int MAX_BONES = 200;" \
		"const int MAX_BONE_INFLUENCE = 4;" \
		"uniform mat4 finalBonesMatrices[MAX_BONES];" \
		"uniform mat4 u_modelMatrix;" \
		"uniform mat4 u_viewMatrix;" \
		"uniform mat4 u_projectionMatrix;" \
		"out vec2 a_texcoord_out;" \
		"void main(void)" \
		"{" \
		"vec4 totalPosition = vec4(0.0f);" \
		"for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)" \
		"{" \
		"if(boneIds[i] == -1)" \
		"continue;" \
		"if(boneIds[i]>=MAX_BONES)" \
		"{" \
		"totalPosition = vec4(a_position);" \
		"break;" \
		"}" \
		"vec4 localPosition = finalBonesMatrices[boneIds[i]] * a_position;" \
		"totalPosition += localPosition * weights[i];" \
		"}" \
		"gl_Position = u_projectionMatrix*u_viewMatrix*u_modelMatrix * totalPosition;" \
		"a_texcoord_out = a_texcoord;" \
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
				myuninitialize();

			}
		}
	}


	// Fragment Shader
	const GLchar* fragmentShaderSourceCode =
		"#version 460 core" \
		"\n" \
		"in vec2 a_texcoord_out;" \
		"uniform sampler2D texture_diffuse1;" \
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"FragColor = texture(texture_diffuse1, a_texcoord_out);" \
		"}";
	GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);
	glCompileShader(fragmentShaderObject);

	status = 0;
	infoLogLength = 0;
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
				myuninitialize();

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
	glBindAttribLocation(shaderProgramObject, ASM_ATTRIBUTE_TEXTURE0, "a_texcoord");
	glBindAttribLocation(shaderProgramObject, ASM_BONEIDS, "boneIds");
	glBindAttribLocation(shaderProgramObject, ASM_BONE_WEIGHTS, "weights");

	// link the
	glLinkProgram(shaderProgramObject);

	// Posted Linked Retrieving /Getting uniform location  from the shader program object.
	modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_modelMatrix");
	viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_viewMatrix");
	perspectiveProjectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_projectionMatrix");
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
				myuninitialize();

			}
		}
	}

	mdl.loadModel(model);
	danceAnimation = Animation(model, &mdl, animation);
	danceAnimation2 = Animation(model, &mdl, nextAnimation);

	animator = Animator(&danceAnimation);





	mdl.printMeshDetails(mdl.meshes);
	auto transforms = animator.GetFinalBoneMatrices();
	debugTransforms(transforms);


	glEnable(GL_TEXTURE_2D);

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


void resize(int width, int height)
{
	// code
	if (height == 0)
		height = 1;  //to avoid divide by 0 in future code
	glViewport(0, 0, width, height);

	perspectiveProjectionMatrix = perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

BOOL autoBlendAnimations = true;
bool startAutoBlendAnimation= false;
GLfloat autoBlendSpeed = 0.001f;
GLfloat autoBlendWait = 1.5f;
GLfloat autoBlendController = 0.0f;
ImVec2 availableSpaceProperties;
void imGUI() 
{

	//code

	ImGuiIO& io = ImGui::GetIO();
	
	// Start ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::DockSpaceOverViewport();
	animationNames = danceAnimation.GetAnimationNames();


	// Other ImGui elements (optional)
	ImGui::Begin("Properties");
	 availableSpaceProperties = ImGui::GetContentRegionAvail();
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::Separator();


	ImGui::Text("Animations");
	if (ImGui::BeginCombo("Animations : A", animationNames[animation].c_str())) {
		for (int i = 0; i < animationNames.size(); ++i) {
			const bool isSelected = (animation == i);
			if (ImGui::Selectable(animationNames[i].c_str(), isSelected))
			{
				animation = i;
				danceAnimation = Animation(model, &mdl, animation);
			}


			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	if (ImGui::BeginCombo("Animations : B", animationNames[nextAnimation].c_str())) {
		for (int i = 0; i < animationNames.size(); ++i) {
			const bool isSelected = (nextAnimation == i);
			if (ImGui::Selectable(animationNames[i].c_str(), isSelected)) {
				danceAnimation2 = Animation(model, &mdl, nextAnimation);
				nextAnimation = i;
			}

			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::SliderFloat("BlendFactor", &animationBlendFactor, 0.0f, 1.0f);
	ImGui::Checkbox("Auto Blend animation", &startAutoBlendAnimation);

	if (startAutoBlendAnimation)
	{



	if (autoBlendAnimations) {
		if (animationBlendFactor <= 1.0f)
		{
			animationBlendFactor += autoBlendSpeed;
		}
		autoBlendController += autoBlendSpeed;

		if (autoBlendController >= autoBlendWait)
			autoBlendAnimations = false;
	}


	if (!autoBlendAnimations) {
		if (animationBlendFactor >= 0.0f)
			animationBlendFactor -= autoBlendSpeed;
		
		autoBlendController -= autoBlendSpeed;
		if (autoBlendController <= -autoBlendWait)
			autoBlendAnimations = true;
	}

	}
	
    
	ImGui::Separator();
	ImGui::Text("Transformations:");
	ImGui::SliderFloat("TranslateX", &translatex, -10.0f, 10.0f);
	ImGui::SliderFloat("TranslateY", &translatey, -10.0f, 10.0f);
	ImGui::SliderFloat("TranslateZ", &translatez, -10.0f, 10.0f);
	ImGui::SliderFloat("RotationX", &angleCube, 0.0f, 360.0f);
	ImGui::SliderFloat("Scale", &scalevalue, 0.0f, 10.0f);
	ImGui::Checkbox("Start Rotation", &startRotation);
	
	if (angleCube >= 360.0f)
		angleCube = 0.0f;

	if (startRotation)
		angleCube += 0.01f;

	ImGui::End();



}


void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//code

	// initialize Imgui
	imGUI();

	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::Begin("Scene");

	// Get available space
	ImVec2 availableSpace = ImGui::GetContentRegionAvail();
	
	// Calculate half of available space for centering

	float halfWidth = availableSpace.x / 2.0f;
	float halfHeight = availableSpace.y / 2.0f;
	glViewport(availableSpaceProperties.x, 0, availableSpace.x, availableSpace.y);
	// Set 
	//ImGui::SetCursorPos(halfAvailableSpace);

	// Use The Shader Program Object
	glUseProgram(shaderProgramObject);
	float currentTimeBase = 0.0f;
	float currentTimeLayered = 0.0f;
	float currentFrame = getTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	//animator.UpdateAnimation(deltaTime, &danceAnimation);
	animator.BlendTwoAnimations(&danceAnimation, &danceAnimation2, animationBlendFactor, deltaTime ,currentTimeBase, currentTimeLayered);

	// Model-View-Projection Matrices
	mat4 translationMatrix = mat4(1.0);
	mat4 rotation = mat4(1.0);
	mat4 view = mat4(1.0);
	mat4 modelMatrix = mat4(1.0);
	mat4 scaleMatrix = mat4(1.0);
	mat4 modelViewProjectionMatrix = mat4(1.0);

	modelMatrix = translate(modelMatrix, vec3(translatex, translatey, translatez));
	modelMatrix = rotate(modelMatrix, angleCube, vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(scalevalue, scalevalue, scalevalue));

	glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(perspectiveProjectionMatrixUniform, 1, GL_FALSE, glm::value_ptr(perspectiveProjectionMatrix));

	animator.printFinalMatrices();
	auto transforms = animator.GetFinalBoneMatrices();
	for (int i = 0; i < transforms.size(); ++i)
		glUniformMatrix4fv(glGetUniformLocation(shaderProgramObject, ("finalBonesMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(transforms[i]));

	for (unsigned int i = 0; i < mdl.meshes.size(); i++) {
		if (i > 0) {

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mdl.meshes[i].textures[0].id);
			glUniform1i(glGetUniformLocation(shaderProgramObject, "texture_diffuse1"), 0);
			glBindVertexArray(mdl.meshes[i].VAO);

			glDrawElements(GL_TRIANGLES, (mdl.meshes[i].indices.size()), GL_UNSIGNED_INT, 0);

			glBindVertexArray(0);

		}
	}

	glUseProgram(0);


	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


	SwapBuffers(ghdc);
}


void update(void)
{
	if (checkAnimation != animation)
	{
	danceAnimation = Animation(model, &mdl, animation);
	animator = Animator(&danceAnimation);
	
	checkAnimation = animation;
	}

	if (checkNextAnimation != nextAnimation)
	{
		danceAnimation2 = Animation(model, &mdl, nextAnimation);
		animator = Animator(&danceAnimation2);

		checkNextAnimation = nextAnimation;
	}



	// Swap buffers
	//SwapBuffers(ghdc);
}

void myuninitialize(void)
{
	// Function declarations
	void ToggleFullscreen(void);

	// Code
	if (gbFullscreen)
	{
		ToggleFullscreen();
	}

	if (VBO_SQUARE_POSITION)
	{
		glDeleteBuffers(1, &VBO_SQUARE_POSITION);
		VBO_SQUARE_POSITION = 0;
	}

	if (VAO_SQUARE)
	{
		glDeleteVertexArrays(1, &VAO_SQUARE);
		VAO_SQUARE = 0;
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

	if (wglGetCurrentContext() == ghrc)
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

