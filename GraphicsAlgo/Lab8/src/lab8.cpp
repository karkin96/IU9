#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <iostream>
#include <fstream>
#include <string>

#define MAX_SCALE 3.275
#define MATH_PI 3.141593

GLuint Program;
GLint  Attrib_vertex;
GLint  Unif_color;
GLuint VBO;
bool carcas_enabled = false;

GLfloat rotate_x = 0.0;
GLfloat rotate_y = 0.0;
GLfloat scale = 2.0;
size_t partition = 20;

unsigned int cubeOffset, baseOffset, wallsOffset, capOffset;

void freeVBO();
void initVBO();

size_t bufferSize;

struct Vec3f {
	GLfloat x, y, z;
	Vec3f() : x(0), y(0), z(0) {};
	Vec3f(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {};
} *drawingBuffer;


GLfloat cubeCoord[][3] = {
	{ 1.0f, 1.0f, 1.0f },
	{ -1.0f, 1.0f, 1.0f },
	{ -1.0, -1.0, 1.0 },
	{ 1.0, -1.0, 1.0 },
	// left face
	{ -1.0, 1.0, 1.0 },
	{ -1.0, 1.0, -1.0 },
	{ -1.0, -1.0, -1.0 },
	{ -1.0, -1.0, 1.0 },
	// back face
	{ 1.0, 1.0, -1.0 },
	{ 1.0, -1.0, -1.0 },
	{ -1.0, -1.0, -1.0 },
	{ -1.0, 1.0, -1.0 },
	// top face
	{ -1.0, 1.0, -1.0 },
	{ -1.0, 1.0, 1.0 },
	{ 1.0, 1.0, 1.0 },
	{ 1.0, 1.0, -1.0 },
	// right face
	{ 1.0, 1.0, 1.0 },
	{ 1.0, -1.0, 1.0 },
	{ 1.0, -1.0, -1.0 },
	{ 1.0, 1.0, -1.0 },
	// buttom face
	{ -1.0, -1.0, -1.0 },
	{ 1.0, -1.0, -1.0 },
	{ 1.0, -1.0, 1.0 },
	{ -1.0, -1.0, 1.0 }
};

void initDrawingBuffer() {
	cubeOffset = 0;
	baseOffset = 24;
	wallsOffset = baseOffset + (partition + 2);
	capOffset = wallsOffset + (partition + 1) * 2;
	bufferSize = capOffset + partition + 2;

	float x0 = 0.0f, y0 = -1.0f, z0 = 0.0f;
	float a = 1.0f, b = 1.5f, h = 2.0f;
	drawingBuffer = new Vec3f[bufferSize];

	// cube coordinates
	for (int i = 0; i < baseOffset; i++) 
		drawingBuffer[cubeOffset + i] = Vec3f(cubeCoord[i][0] - 2.4, cubeCoord[i][1], cubeCoord[i][2]);
	
	float dphi = 2 * (float)MATH_PI / (partition - 1);
	float phi;

	// clylinder cap and base coordinates
	drawingBuffer[baseOffset] = Vec3f(x0, y0, z0);
	drawingBuffer[capOffset] = Vec3f(x0, y0 + h, z0);
	for (int i = 1; i <= partition + 1; i++) {
		phi = ((i - 1)%partition) * dphi;

		drawingBuffer[baseOffset + i] = Vec3f(x0 + a * (float)cos(phi), y0, z0 + b * (float)sin(phi));
		drawingBuffer[capOffset + i] = Vec3f(x0 + a * (float)cos(phi),	y0 + h,	z0 + b * (float)sin(phi));
	}

	// cylinder walls coordinates
	for (size_t i = 0; i <= 2 * (partition - 1); i += 2) {
		phi = ((i / 2) % partition) * dphi;
		drawingBuffer[wallsOffset + i] = Vec3f(x0 + a * (float)cos(phi), y0, z0 + b * (float)sin(phi));
		drawingBuffer[wallsOffset + i + 1] = Vec3f(x0 + a * (float)cos(phi), y0 + h, z0 + b * (float)sin(phi));
	}	
}

void resizeDrawingBuffer() {
	freeVBO();
	initDrawingBuffer();
	initVBO();
}


void shaderLog(unsigned int shader)
{
	int   infologLen = 0;
	int   charsWritten = 0;
	char *infoLog;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);

	if (infologLen > 1)
	{
		infoLog = new char[infologLen];
		if (infoLog == NULL)
		{
			std::cout << "ERROR: Could not allocate InfoLog buffer\n";
			exit(1);
		}
		glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog);
		std::cout << "InfoLog: " << infoLog << "\n\n\n";
		delete[] infoLog;
	}
}

void initGL()
{
	glClearColor(0, 0, 0, 0);
}

GLuint loadShader(std::string file_path, GLuint shader_type) {
	std::ifstream src_stream(file_path);
	std::string src = "";
	std::string cur_line;
	GLuint id;
	while (std::getline(src_stream, cur_line)) {
		src += cur_line;
	}
	id = glCreateShader(shader_type);
	const char* buffer = src.c_str();
	glShaderSource(id, 1, &buffer, NULL);
	glCompileShader(id);
	return id;
}

void initShader()
{
	GLuint vShader, fShader;

	vShader = loadShader("C:\\tmp\\vshader.glsl", GL_VERTEX_SHADER);
	fShader = loadShader("C:\\tmp\\fshader.glsl", GL_FRAGMENT_SHADER);

	std::cout << "vertex shader \n";
	shaderLog(vShader);

	std::cout << "fragment shader \n";
	shaderLog(fShader);

	Program = glCreateProgram();
	glAttachShader(Program, vShader);
	glAttachShader(Program, fShader);
	glLinkProgram(Program);

	int link_ok;
	glGetProgramiv(Program, GL_LINK_STATUS, &link_ok);
	if (!link_ok)
	{
		std::cout << "error attach shaders \n";
		return;
	}

	const char* attr_name = "coord";
	Attrib_vertex = glGetAttribLocation(Program, attr_name);
	if (Attrib_vertex == -1)
	{
		std::cout << "could not bind attrib " << attr_name << std::endl;
		return;
	}

	const char* unif_name = "color";
	Unif_color = glGetUniformLocation(Program, unif_name);
	if (Unif_color == -1)
	{
		std::cout << "could not bind uniform " << unif_name << std::endl;
		return;
	}
}

void initVBO()
{
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//glBufferData(GL_ARRAY_BUFFER, 24*3*sizeof(float) + 4, cubeCoord, GL_STATIC_DRAW);
	//float *buf = (float*)drawingBuffer;
	//std::cout << "---=" << buf[0] << " " << " " << buf[1] << " " << buf[2] << std::endl;
	glBufferData(GL_ARRAY_BUFFER, (bufferSize)*sizeof(Vec3f), drawingBuffer, GL_STATIC_DRAW);
}

void freeShader()
{
	glUseProgram(0);
	glDeleteProgram(Program);
}

void freeVBO()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO);
}

void resizeWindow(int width, int height)
{
	glViewport(0, 0, width, height);
}

static float red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
static float green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
static float blue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
static float yellow[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
static float pink[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
static float aqua_marine[4] = { 0.0f, 1.0f, 1.0f, 1.0f };

void drawCube() {
	glLoadIdentity();
	glScalef(2.0f, 2.0f, 2.0f);
	glTranslatef(-4.5f, 0.0f, -3.0f);
	glRotatef(-30, 1.0, 0.0, 0.0);
	glRotatef(-20, 0.0, 1.0, 0.0);

	glUniform4fv(Unif_color, 1, red);
	glDrawArrays(GL_QUADS, 0, 4);

	glUniform4fv(Unif_color, 1, green);
	glDrawArrays(GL_QUADS, 4, 4);

	glUniform4fv(Unif_color, 1, blue);
	glDrawArrays(GL_QUADS, 8, 4);

	glUniform4fv(Unif_color, 1, yellow);
	glDrawArrays(GL_QUADS, 12, 4);

	glUniform4fv(Unif_color, 1, pink);
	glDrawArrays(GL_QUADS, 16, 4);

	glUniform4fv(Unif_color, 1, aqua_marine);
	glDrawArrays(GL_QUADS, 20, 4);
}

void drawCylinder() {
	glLoadIdentity();
	glScalef(scale, scale, scale);
	glRotatef(rotate_x, 1.0, 0.0, 0.0);
	glRotatef(rotate_y, 0.0, 1.0, 0.0);

	glUniform4fv(Unif_color, 1, blue);
	glDrawArrays(GL_TRIANGLE_FAN, baseOffset, wallsOffset - baseOffset);
	glUniform4fv(Unif_color, 1, red);
	glDrawArrays(GL_QUAD_STRIP, wallsOffset, capOffset - wallsOffset - 1);
	glUniform4fv(Unif_color, 1, blue);
	glDrawArrays(GL_TRIANGLE_FAN, capOffset, wallsOffset - baseOffset);
}

void render()
{
	glUseProgram(Program);
	glEnableVertexAttribArray(Attrib_vertex);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(Attrib_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	drawCube();
	drawCylinder();

	glDisableVertexAttribArray(Attrib_vertex);
	glUseProgram(0);
}

void init_projection(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);

	GLfloat matrix_3p[16] = {
		1.f,    0.00f,  0.0f,   -0.6f,
		0.0f,   1.0f,   0.0f,   -0.6f,
		0.0f,   0.f,    1.0f,   0.6f,
		0.0f,   0.0f,   0.00f,     1.f
	};

	glLoadMatrixf(matrix_3p);
	glOrtho(-(GLfloat)width / 64.0f, (GLfloat)width / 64.0f, -(GLfloat)height / 64.0f, (GLfloat)height / 64.0f, -10, 10);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS)
		return;
	switch (key) {
	case GLFW_KEY_UP:
		rotate_x += 10;
		break;
	case GLFW_KEY_DOWN:
		rotate_x -= 10;
		break;
	case GLFW_KEY_LEFT:
		rotate_y -= 10;
		break;
	case GLFW_KEY_RIGHT:
		rotate_y += 10;
		break;
	case GLFW_KEY_P:
		if (scale*1.1f<MAX_SCALE)
			scale *= 1.1f;
		break;
	case GLFW_KEY_M:
		scale /= 1.1f;
		break;
	case GLFW_KEY_COMMA:
		if (partition > 5) {
			partition -= 5;
			resizeDrawingBuffer();
		}
		break;
	case GLFW_KEY_PERIOD:
		partition += 5;
		resizeDrawingBuffer();
		break;
	case GLFW_KEY_C:
		carcas_enabled = !carcas_enabled;
		break;
	}
}

static void error_callback(int error, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

int main(int argc, char **argv)
{
	GLFWwindow* window;
	//std::cout << sizeof(Vec3f) << std::endl;
	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(640, 480, "Lab 8", NULL, NULL);
	if (!window)
	{
		std::cerr << "Failed to open GLFW window" << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	std::cout << glGetString(GL_VERSION) << std::endl;

	glEnable(GL_DEPTH_TEST);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, init_projection);

	init_projection(window, 640, 480);

	GLenum glew_status = glewInit();
	if (GLEW_OK != glew_status)
	{
		std::cout << "Error: " << glewGetErrorString(glew_status) << "\n";
		return 1;
	}

	if (!GLEW_VERSION_2_0)
	{
		std::cout << "No support for OpenGL 2.0 found\n";
		return 1;
	}

	initDrawingBuffer();
	initGL();
	initVBO();
	initShader();

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glPolygonMode(GL_FRONT_AND_BACK, carcas_enabled ? GL_LINE : GL_FILL);

		render();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	freeShader();
	freeVBO();
	delete[] drawingBuffer;
}