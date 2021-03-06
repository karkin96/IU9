#include <iostream>

#include <EllipticalCylinder.h>
#include <Scene.h>
#include <SOIL.h>

#define MAX_SCALE 3.275

float start_time;
bool side = true;

EllipticalCylinder *cylinder;
Texture *tex1, *tex2;
Scene scene;

void init_projection(GLFWwindow*, int, int);
void init_scene(char*);
void animateCylinder();
void switch_light(int);

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS)
		return;
	switch (key) {
	case GLFW_KEY_UP:
		scene.rotate_x += 10;
		break;
	case GLFW_KEY_DOWN:
		scene.rotate_x -= 10;
		break;
	case GLFW_KEY_LEFT:
		scene.rotate_y -= 10;
		break;
	case GLFW_KEY_RIGHT:
		scene.rotate_y += 10;
		break;
	case GLFW_KEY_C:
		scene.carcas_enabled = !scene.carcas_enabled;
		break;
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GL_TRUE);
		break;
	case GLFW_KEY_P:
		if (scene.scale*1.1f<MAX_SCALE)
			scene.scale *= 1.1f;
		break;
	case GLFW_KEY_M:
		scene.scale /= 1.1f;
		break;
	case GLFW_KEY_T:
		scene.texture_enabled = !scene.texture_enabled;
		break;
	case GLFW_KEY_COMMA:
		if (scene.partition>5) {
			scene.partition -= 5;
			cylinder->setPartition(scene.partition);
		}
		break;
	case GLFW_KEY_PERIOD: 
		scene.partition += 5;
		cylinder->setPartition(scene.partition);
		break;
	case GLFW_KEY_A:
		scene.animation_enabled = !scene.animation_enabled;
		break;
	case GLFW_KEY_0:
		switch_light(0);
		break;
	case GLFW_KEY_1:
		switch_light(1);
		break;
	case GLFW_KEY_2:
		switch_light(2);
		break;
	case GLFW_KEY_3:
		switch_light(3);
		break;
	case GLFW_KEY_4:
		switch_light(4);
		break;
	case GLFW_KEY_5:
		switch_light(5);
		break;
	case GLFW_KEY_6:
		switch_light(6);
		break;
	case GLFW_KEY_7:
		switch_light(7);
		break;
	default:
		break;
	}
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

int main(int argc, char *argv[])
{
	GLFWwindow* window; 
	glfwSetErrorCallback(error_callback);
	if (argc != 3) {
		cerr << "Invalid number of argments" << endl;
		cout << "Ussage: Lab6 -c [config_file]" << endl;
		return -1;
	}
	else if (argv[1][0] != '-' && argv[1][1]!='c') {
		cerr << "Invalid parameter" << endl;
		cout << "Ussage: Lab6 -c [config_file]" << endl;
		return -1;
	}

	if (!glfwInit()) {
		cerr << "Failed to initialize GLFW" << endl;
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(640, 480, "Lab 6", NULL, NULL);
	if (!window)
	{
		cerr << "Failed to open GLFW window" << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	cout << glGetString(GL_VERSION) << endl;

	glEnable(GL_DEPTH_TEST);

	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, init_projection);

	init_projection(window, 640, 480);
	init_scene(argv[2]);

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);

		glLoadIdentity();
		glScalef(2.0f, 2.0f, 2.0f);
		glTranslatef(-4.5f, 0.0f, -3.0f);
		glRotatef(-30, 1.0, 0.0, 0.0);
		glRotatef(-20, 0.0, 1.0, 0.0);
		drawCube();
		
		glLoadIdentity();
		animateCylinder();
		glScalef(scene.scale, scene.scale, scene.scale);
		glRotatef(scene.rotate_x, 1.0, 0.0, 0.0);
		glRotatef(scene.rotate_y, 0.0, 1.0, 0.0);

		if (scene.animation_enabled) {
			animateCylinder();
			cylinder->draw(scene.current_time);
		}
		else
			cylinder->draw(0);
		if (scene.texture_enabled)	
			glEnable(GL_TEXTURE_2D);
		else 
			glDisable(GL_TEXTURE_2D);

		glPolygonMode(GL_FRONT_AND_BACK, scene.carcas_enabled ? GL_LINE : GL_FILL);

		glfwSwapBuffers(window);
		glfwPollEvents();

	}
	scene.save();
	glfwTerminate();
	delete cylinder;
	exit(EXIT_SUCCESS);
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

void init_scene(char *settings_file) {
	scene.settings_file = settings_file;
	scene.load();
	cylinder = new EllipticalCylinder(1.0f, 1.5f, 2.0f, scene.partition);

	initGloabalLight(scene.global_light);
	setMaterial(scene.materials[3]);
	for (int i = 0; i < scene.local_lights.size(); i++)
		initLight(scene.local_lights[i]);

	for (int i = 0; i < scene.textures.size(); i++) {
		scene.textures[i].load();
	}

	tex1 = &scene.textures[0], tex2 = &scene.textures[1];
}

void animateCylinder() {
	if (!scene.animation_enabled)
		return;
	float speed = 0.005f;
	if (scene.current_time + speed > 1 || scene.current_time - speed < 0) 
		side = !side;
	if (side)
		scene.current_time += speed;
	else 
		scene.current_time -= speed;

}

void switch_light(int i) {
	if (scene.local_lights.size() > i && i >= 0) {
		if (scene.local_lights[i].enabled) {
			scene.local_lights[i].code = switch_code(i);
			glDisable(scene.local_lights[i].code);
			scene.local_lights[i].enabled = false;
		}
		else {
			scene.local_lights[i].code = switch_code(i);
			glEnable(scene.local_lights[i].code);
			scene.local_lights[i].enabled = true;
		}
		return;
	}
	cerr << "Incorrect index of light. Maximum count of lights is eight." << endl;
}