#include "Window.h"

Window::Window()
{
	width = 800;
	height = 600;
	muevex = 2.0f;

	// Inicializar variables del Rover
	articulacion1 = 0.0f; articulacion2 = 0.0f; articulacion3 = 0.0f; articulacion4 = 0.0f;
	articulacion5 = 0.0f; articulacion6 = 0.0f; articulacion7 = 0.0f;
	articulacion8 = 0.0f; articulacion9 = 0.0f; articulacion10 = 0.0f;
	avanceLlantas = 0.0f;

	for (size_t i = 0; i < 1024; i++)
	{
		keys[i] = 0;
	}
}

Window::Window(GLint windowWidth, GLint windowHeight)
{
	width = windowWidth;
	height = windowHeight;
	muevex = 2.0f;

	// Inicializar variables del Rover
	articulacion1 = 0.0f; articulacion2 = 0.0f; articulacion3 = 0.0f; articulacion4 = 0.0f;
	articulacion5 = 0.0f; articulacion6 = 0.0f; articulacion7 = 0.0f;
	articulacion8 = 0.0f; articulacion9 = 0.0f; articulacion10 = 0.0f;
	avanceLlantas = 0.0f;

	for (size_t i = 0; i < 1024; i++)
	{
		keys[i] = 0;
	}
}

int Window::Initialise()
{
	//Inicialización de GLFW
	if (!glfwInit())
	{
		printf("Falló inicializar GLFW");
		glfwTerminate();
		return 1;
	}
	//Asignando variables de GLFW y propiedades de ventana
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//CREAR VENTANA
	mainWindow = glfwCreateWindow(width, height, "Practica07:Iluminacion I", NULL, NULL);

	if (!mainWindow)
	{
		printf("Fallo en crearse la ventana con GLFW");
		glfwTerminate();
		return 1;
	}
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);
	glfwMakeContextCurrent(mainWindow);
	createCallbacks();
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		printf("Falló inicialización de GLEW");
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, bufferWidth, bufferHeight);
	glfwSetWindowUserPointer(mainWindow, this);

	return 0;
}

void Window::createCallbacks()
{
	glfwSetKeyCallback(mainWindow, ManejaTeclado);
	glfwSetCursorPosCallback(mainWindow, ManejaMouse);
}

GLfloat Window::getXChange()
{
	GLfloat theChange = xChange;
	xChange = 0.0f;
	return theChange;
}

GLfloat Window::getYChange()
{
	GLfloat theChange = yChange;
	yChange = 0.0f;
	return theChange;
}

void Window::ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	if (key == GLFW_KEY_Y)
	{
		theWindow->muevex += 1.0;
	}
	if (key == GLFW_KEY_U)
	{
		theWindow->muevex -= 1.0;
	}

	// --- CONTROLES DEL ROVER (Traídos de la Práctica 5) ---
	float vel_rover = 2.0f;

	// Brazo (Teclas Q/A, W/S)
	if (key == GLFW_KEY_Q) { theWindow->articulacion1 += vel_rover; }
	if (key == GLFW_KEY_A) { theWindow->articulacion1 -= vel_rover; }
	if (key == GLFW_KEY_W) { theWindow->articulacion2 += vel_rover; }
	if (key == GLFW_KEY_S) { theWindow->articulacion2 -= vel_rover; }

	// Patas de Suspensión (Límites 45 grados)
	if (key == GLFW_KEY_F) { if (theWindow->articulacion5 < 45.0f) theWindow->articulacion5 += vel_rover; }
	if (key == GLFW_KEY_V) { if (theWindow->articulacion5 > -45.0f) theWindow->articulacion5 -= vel_rover; }
	if (key == GLFW_KEY_G) { if (theWindow->articulacion6 < 45.0f) theWindow->articulacion6 += vel_rover; }
	if (key == GLFW_KEY_B) { if (theWindow->articulacion6 > -45.0f) theWindow->articulacion6 -= vel_rover; }
	if (key == GLFW_KEY_H) { if (theWindow->articulacion7 < 45.0f) theWindow->articulacion7 += vel_rover; }
	if (key == GLFW_KEY_N) { if (theWindow->articulacion7 > -45.0f) theWindow->articulacion7 -= vel_rover; }
	if (key == GLFW_KEY_J) { if (theWindow->articulacion8 < 45.0f) theWindow->articulacion8 += vel_rover; }
	if (key == GLFW_KEY_M) { if (theWindow->articulacion8 > -45.0f) theWindow->articulacion8 -= vel_rover; }
	if (key == GLFW_KEY_K) { if (theWindow->articulacion9 < 45.0f) theWindow->articulacion9 += vel_rover; }
	if (key == GLFW_KEY_COMMA) { if (theWindow->articulacion9 > -45.0f) theWindow->articulacion9 -= vel_rover; }
	if (key == GLFW_KEY_L) { if (theWindow->articulacion10 < 45.0f) theWindow->articulacion10 += vel_rover; }
	if (key == GLFW_KEY_PERIOD) { if (theWindow->articulacion10 > -45.0f) theWindow->articulacion10 -= vel_rover; }

	// Avance de Ruedas
	if (key == GLFW_KEY_SPACE) { theWindow->avanceLlantas += 5.0f; }
	// ------------------------------------------------------


	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			theWindow->keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			theWindow->keys[key] = false;
		}
	}
}

void Window::ManejaMouse(GLFWwindow* window, double xPos, double yPos)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (theWindow->mouseFirstMoved)
	{
		theWindow->lastX = xPos;
		theWindow->lastY = yPos;
		theWindow->mouseFirstMoved = false;
	}

	theWindow->xChange = xPos - theWindow->lastX;
	theWindow->yChange = theWindow->lastY - yPos;

	theWindow->lastX = xPos;
	theWindow->lastY = yPos;
}

Window::~Window()
{
	glfwDestroyWindow(mainWindow);
	glfwTerminate();
}