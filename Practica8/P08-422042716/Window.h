#pragma once
#include <stdio.h>
#include <glew.h>
#include <glfw3.h>

class Camera;

class Window
{
public:
	Window();
	Window(GLint windowWidth, GLint windowHeight);
	int Initialise();
	GLfloat getBufferWidth() { return bufferWidth; }
	GLfloat getBufferHeight() { return bufferHeight; }
	GLfloat getXChange();
	GLfloat getYChange();
	GLfloat getmuevex() { return muevex; }
	bool getShouldClose() { return glfwWindowShouldClose(mainWindow); }
	bool* getsKeys() { return keys; }
	void swapBuffers() { glfwSwapBuffers(mainWindow); }

	// Getters del Rover
	GLfloat getarticulacion1() { return articulacion1; }
	GLfloat getarticulacion2() { return articulacion2; }
	GLfloat getarticulacion3() { return articulacion3; }
	GLfloat getarticulacion4() { return articulacion4; }
	GLfloat getarticulacion5() { return articulacion5; }
	GLfloat getarticulacion6() { return articulacion6; }
	GLfloat getarticulacion7() { return articulacion7; }
	GLfloat getarticulacion8() { return articulacion8; }
	GLfloat getarticulacion9() { return articulacion9; }
	GLfloat getarticulacion10() { return articulacion10; }
	GLfloat getAvanceLlantas() { return avanceLlantas; }

	// Estado de las luces (Z: Holocron, X: pinza del Rover)
	bool getLuzHolocron() { return luzHolocron; }
	bool getLuzPinza() { return luzPinza; }

    void ActualizarEscena(GLfloat paso, GLfloat deltaCamara, Camera& camara);
    bool getLucesNave() const { return lucesNave; }
    bool getMotor1() const { return motor1; }
    bool getMotor2() const { return motor2; }
    GLfloat getRotObjX() const { return rotX; }
    GLfloat getRotObjY() const { return rotY; }
    GLfloat getRotObjZ() const { return rotZ; }
    GLfloat getMovRoverX() const { return roverX; }
    GLfloat getMovRoverZ() const { return roverZ; }
    GLfloat getRotRoverY() const { return roverY; }
	~Window();
private:
    bool lucesNave = true, motor1 = true, motor2 = true;
    GLfloat rotX = 0, rotY = 0, rotZ = 0;
    GLfloat roverX = 0, roverZ = 0, roverY = 0;
	GLFWwindow* mainWindow;
	GLint width, height;
	bool keys[1024];
	GLint bufferWidth, bufferHeight;
	void createCallbacks();
	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange;
	GLfloat yChange;
	GLfloat muevex;
	bool mouseFirstMoved;

	// Variables del Rover
	GLfloat articulacion1, articulacion2, articulacion3, articulacion4;
	GLfloat articulacion5, articulacion6, articulacion7, articulacion8, articulacion9, articulacion10;
	GLfloat avanceLlantas;

	// Luces encendidas (true) o apagadas (false)
	bool luzHolocron, luzPinza;

	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);
};