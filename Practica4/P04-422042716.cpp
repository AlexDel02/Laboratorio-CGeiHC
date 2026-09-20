/*Práctica 4: Modelado Jerárquico.
Se implementa el uso de matrices adicionales para almacenar información de transformaciones geométricas que se quiere
heredar entre diversas instancias para que estén unidas
*/
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <glew.h>
#include <glfw3.h>
//glm
#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
#include <gtc\random.hpp>
//clases para dar orden y limpieza al còdigo
#include "Mesh.h"
#include "Shader.h"
#include "Sphere.h"
#include "Window.h"
#include "Camera.h"

using std::vector;

//Dimensiones de la ventana
const float toRadians = 3.14159265f / 180.0; //grados a radianes
const float PI = 3.14159265f;
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;
Camera camera;
Window mainWindow;
vector<Mesh*> meshList;
vector<Shader> shaderList;

//Vertex Shader
static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";
Sphere sp = Sphere(1.0, 20, 20); //recibe radio, slices, stacks

// --- VARIABLES PARA CONTROL INDEPENDIENTE ---
// 0: Base, 1: Hombro, 2: Codo, 3: Muñeca, 4: Pinza
GLfloat rotArticulacion[5] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

// 0: Frontal Izq, 1: Media Izq, 2: Trasera Izq, 3: Frontal Der, 4: Media Der, 5: Trasera Der
GLfloat rotRuedas[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

// --- ROTACIÓN DE LA VOYAGER ---
GLfloat rotVoyager = 0.0f;


void CrearCubo()
{
	unsigned int cubo_indices[] = {
		// front
		0, 1, 2, 2, 3, 0,
		// right
		1, 5, 6, 6, 2, 1,
		// back
		7, 6, 5, 5, 4, 7,
		// left
		4, 0, 3, 3, 7, 4,
		// bottom
		4, 5, 1, 1, 0, 4,
		// top
		3, 2, 6, 6, 7, 3
	};

	GLfloat cubo_vertices[] = {
		// front
		-0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		// back
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f
	};
	Mesh* cubo = new Mesh();
	cubo->CreateMesh(cubo_vertices, cubo_indices, 24, 36);
	meshList.push_back(cubo);
}

void CrearPiramideTriangular()
{
	unsigned int indices_piramide_triangular[] = {
			0,1,2, 1,3,2, 3,0,2, 1,0,3
	};
	GLfloat vertices_piramide_triangular[] = {
		-0.5f, -0.5f,0.0f,	//0
		0.5f,-0.5f,0.0f,	//1
		0.0f,0.5f, -0.25f,	//2
		0.0f,-0.5f,-0.5f,	//3
	};
	Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices_piramide_triangular, indices_piramide_triangular, 12, 12);
	meshList.push_back(obj1);
}

void CrearCilindro(int res, float R) {
	int n, i;
	GLfloat dt = 2 * PI / res, x, z, y = -0.5f;

	vector<GLfloat> vertices;
	vector<unsigned int> indices;

	for (n = 0; n <= (res); n++) {
		if (n != res) {
			x = R * cos((n)*dt);
			z = R * sin((n)*dt);
		}
		else {
			x = R * cos((0) * dt);
			z = R * sin((0) * dt);
		}
		for (i = 0; i < 6; i++) {
			switch (i) {
			case 0: vertices.push_back(x); break;
			case 1: vertices.push_back(y); break;
			case 2: vertices.push_back(z); break;
			case 3: vertices.push_back(x); break;
			case 4: vertices.push_back(0.5); break;
			case 5: vertices.push_back(z); break;
			}
		}
	}

	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0: vertices.push_back(x); break;
			case 1: vertices.push_back(-0.5f); break;
			case 2: vertices.push_back(z); break;
			}
		}
	}

	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0: vertices.push_back(x); break;
			case 1: vertices.push_back(0.5); break;
			case 2: vertices.push_back(z); break;
			}
		}
	}

	for (i = 0; i < vertices.size(); i++) indices.push_back(i);

	Mesh* cilindro = new Mesh();
	cilindro->CreateMeshGeometry(vertices, indices, vertices.size(), indices.size());
	meshList.push_back(cilindro);
}

void CrearCono(int res, float R) {
	int n, i;
	GLfloat dt = 2 * PI / res, x, z, y = -0.5f;

	vector<GLfloat> vertices;
	vector<unsigned int> indices;

	vertices.push_back(0.0);
	vertices.push_back(0.5);
	vertices.push_back(0.0);

	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0: vertices.push_back(x); break;
			case 1: vertices.push_back(y); break;
			case 2: vertices.push_back(z); break;
			}
		}
	}
	vertices.push_back(R * cos(0) * dt);
	vertices.push_back(-0.5);
	vertices.push_back(R * sin(0) * dt);

	for (i = 0; i < res + 2; i++) indices.push_back(i);

	Mesh* cono = new Mesh();
	cono->CreateMeshGeometry(vertices, indices, vertices.size(), res + 2);
	meshList.push_back(cono);
}

void CrearPiramideCuadrangular()
{
	vector<unsigned int> piramidecuadrangular_indices = {
		0,3,4, 3,2,4, 2,1,4, 1,0,4, 0,1,2, 0,2,4
	};
	vector<GLfloat> piramidecuadrangular_vertices = {
		0.5f,-0.5f,0.5f, 0.5f,-0.5f,-0.5f, -0.5f,-0.5f,-0.5f, -0.5f,-0.5f,0.5f, 0.0f,0.5f,0.0f,
	};
	Mesh* piramide = new Mesh();
	piramide->CreateMeshGeometry(piramidecuadrangular_vertices, piramidecuadrangular_indices, 15, 18);
	meshList.push_back(piramide);
}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

int main()
{
	mainWindow = Window(800, 600);
	mainWindow.Initialise();

	CrearCubo();                // índice 0 
	CrearPiramideTriangular();  // índice 1 
	CrearCilindro(18, 1.0f);    // índice 2 
	CrearCono(25, 2.0f);        // índice 3 
	CrearPiramideCuadrangular();// índice 4 
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.3f);

	GLuint uniformProjection = 0;
	GLuint uniformModel = 0;
	GLuint uniformView = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(glm::radians(60.0f), mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);

	sp.init();
	sp.load();

	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::mat4 modelaux2(1.0);
	glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);

	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// CONTROLES DE TECLADO 
		bool* keys = mainWindow.getsKeys();

		// Brazo Rover
		if (keys[GLFW_KEY_F]) rotArticulacion[0] += 1.5f;
		if (keys[GLFW_KEY_G]) rotArticulacion[0] -= 1.5f;
		if (keys[GLFW_KEY_H]) rotArticulacion[1] += 1.5f;
		if (keys[GLFW_KEY_J]) rotArticulacion[1] -= 1.5f;
		if (keys[GLFW_KEY_K]) rotArticulacion[2] += 1.5f;
		if (keys[GLFW_KEY_L]) rotArticulacion[2] -= 1.5f;
		if (keys[GLFW_KEY_Z]) rotArticulacion[3] += 1.5f;
		if (keys[GLFW_KEY_X]) rotArticulacion[3] -= 1.5f;
		if (keys[GLFW_KEY_C]) rotArticulacion[4] += 1.5f;
		if (keys[GLFW_KEY_V]) rotArticulacion[4] -= 1.5f;

		// Llantas Rover INDEPENDIENTES 
		if (keys[GLFW_KEY_1]) rotRuedas[0] += 3.0f;
		if (keys[GLFW_KEY_Q]) rotRuedas[0] -= 3.0f;
		if (keys[GLFW_KEY_2]) rotRuedas[1] += 3.0f;
		if (keys[GLFW_KEY_W]) rotRuedas[1] -= 3.0f;
		if (keys[GLFW_KEY_3]) rotRuedas[2] += 3.0f;
		if (keys[GLFW_KEY_E]) rotRuedas[2] -= 3.0f;
		if (keys[GLFW_KEY_4]) rotRuedas[3] += 3.0f;
		if (keys[GLFW_KEY_R]) rotRuedas[3] -= 3.0f;
		if (keys[GLFW_KEY_5]) rotRuedas[4] += 3.0f;
		if (keys[GLFW_KEY_T]) rotRuedas[4] -= 3.0f;
		if (keys[GLFW_KEY_6]) rotRuedas[5] += 3.0f;
		if (keys[GLFW_KEY_Y]) rotRuedas[5] -= 3.0f;

		// Voyager Rotación
		if (keys[GLFW_KEY_U]) rotVoyager += 1.0f;
		if (keys[GLFW_KEY_I]) rotVoyager -= 1.0f;


		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderList[0].useShader();
		uniformModel = shaderList[0].getModelLocation();
		uniformProjection = shaderList[0].getProjectLocation();
		uniformView = shaderList[0].getViewLocation();
		uniformColor = shaderList[0].getColorLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));


		// MODELO 1: ROVER
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-6.0f, 0.0f, -12.0f));
		modelaux = model;

		// CABINA MAGENTA
		model = glm::translate(modelaux, glm::vec3(1.0f, 2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(8.0f, 4.0f, 4.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 0.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[0]->RenderMesh();

		// BASE VERDE 
		model = modelaux;
		model = glm::translate(model, glm::vec3(0.0f, -0.75f, 0.0f));
		modelaux = model;
		model = glm::scale(model, glm::vec3(10.0f, 1.5f, 8.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.0f, 1.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[0]->RenderMesh();

		modelaux2 = modelaux;

		// BRAZO ROBÓTICO
		model = modelaux2;
		model = glm::translate(model, glm::vec3(-3.75f, 1.25f, 3.0f));
		model = glm::rotate(model, glm::radians(rotArticulacion[0]), glm::vec3(0.0f, 1.0f, 0.0f));
		modelaux = model;
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(0.0f, 0.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[0]->RenderMesh();

		model = modelaux;
		model = glm::translate(model, glm::vec3(0.0f, 0.75f, 0.0f));
		model = glm::rotate(model, glm::radians(rotArticulacion[1]), glm::vec3(0.0f, 0.0f, 1.0f));
		modelaux = model;
		glm::mat4 renderEsfera1 = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderEsfera1));
		color = glm::vec3(1.0f, 0.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		model = modelaux;
		model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(-0.25f, 1.5f, 0.0f));
		modelaux = model;
		glm::mat4 renderBrazo1 = glm::scale(model, glm::vec3(0.5f, 3.0f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderBrazo1));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();

		model = modelaux;
		model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0f));
		model = glm::rotate(model, glm::radians(rotArticulacion[2]), glm::vec3(0.0f, 0.0f, 1.0f));
		modelaux = model;
		glm::mat4 renderEsfera2 = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderEsfera2));
		color = glm::vec3(1.0f, 1.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		model = modelaux;
		model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0f));
		modelaux = model;
		glm::mat4 renderBrazo2 = glm::scale(model, glm::vec3(0.4f, 3.0f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderBrazo2));
		color = glm::vec3(0.8f, 0.8f, 0.8f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();

		model = modelaux;
		model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0f));
		model = glm::rotate(model, glm::radians(rotArticulacion[3]), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::rotate(model, glm::radians(rotArticulacion[4]), glm::vec3(0.0f, 1.0f, 0.0f));
		modelaux = model;
		glm::mat4 renderEsfera3 = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderEsfera3));
		color = glm::vec3(1.0f, 0.5f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		model = modelaux;
		model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f));
		glm::mat4 renderPinza = glm::scale(model, glm::vec3(0.6f, 0.8f, 0.6f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderPinza));
		color = glm::vec3(0.2f, 0.2f, 0.2f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[0]->RenderMesh();


		// Ruedas
		for (int lado = -1; lado <= 1; lado += 2) {
			float z_brazo = 4.4f * lado;
			float z_llanta = 5.1f * lado;
			float z_peg = 4.15f * lado;
			float z_hub = 4.75f * lado;
			int idxBase = (lado == -1) ? 0 : 3;

			color = glm::vec3(0.6f, 0.6f, 0.6f); // Gris metálico 
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));

			// ---------------- 1. RUEDA FRONTAL ----------------
			// Eje de anclaje
			glm::mat4 pegF = glm::translate(modelaux2, glm::vec3(-2.5f, -0.75f, z_peg));
			glm::mat4 renderPegF = glm::scale(pegF, glm::vec3(0.3f, 0.3f, 0.5f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderPegF));
			meshList[0]->RenderMesh();

			// Brazo Diagonal Frontal
			glm::mat4 diagF = glm::translate(modelaux2, glm::vec3(-3.25f, -1.625f, z_brazo));
			diagF = glm::rotate(diagF, glm::radians(49.4f), glm::vec3(0.0f, 0.0f, 1.0f));
			glm::mat4 renderDiagF = glm::scale(diagF, glm::vec3(2.3f, 0.3f, 0.3f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderDiagF));
			meshList[0]->RenderMesh();

			// Eje a la llanta Frontal
			glm::mat4 hubF = glm::translate(modelaux2, glm::vec3(-4.0f, -2.5f, z_hub));
			glm::mat4 renderHubF = glm::scale(hubF, glm::vec3(0.3f, 0.3f, 0.7f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderHubF));
			meshList[0]->RenderMesh();

			// Llanta Frontal
			glm::mat4 llantaF = glm::translate(modelaux2, glm::vec3(-4.0f, -2.5f, z_llanta));
			llantaF = glm::rotate(llantaF, glm::radians(rotRuedas[idxBase + 0]), glm::vec3(0.0f, 0.0f, 1.0f));
			llantaF = glm::rotate(llantaF, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 renderLlantaF = glm::scale(llantaF, glm::vec3(1.5f, 1.0f, 1.5f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderLlantaF));
			glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(0.15f, 0.15f, 0.15f)));
			meshList[2]->RenderMeshGeometry();


			// ---------------- 2. RUEDA MEDIA ----------------
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			// Eje de anclaje
			glm::mat4 pegM = glm::translate(modelaux2, glm::vec3(1.0f, -0.75f, z_peg));
			glm::mat4 renderPegM = glm::scale(pegM, glm::vec3(0.3f, 0.3f, 0.5f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderPegM));
			meshList[0]->RenderMesh();

			glm::mat4 horizM = glm::translate(modelaux2, glm::vec3(0.75f, -0.75f, z_brazo));
			glm::mat4 renderHorizM = glm::scale(horizM, glm::vec3(0.5f, 0.3f, 0.3f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderHorizM));
			meshList[0]->RenderMesh();

			glm::mat4 vertM = glm::translate(modelaux2, glm::vec3(0.25f, -1.625f, z_brazo));
			vertM = glm::rotate(vertM, glm::radians(74.05f), glm::vec3(0.0f, 0.0f, 1.0f));
			glm::mat4 renderVertM = glm::scale(vertM, glm::vec3(1.82f, 0.3f, 0.3f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderVertM));
			meshList[0]->RenderMesh();

			glm::mat4 hubM = glm::translate(modelaux2, glm::vec3(0.0f, -2.5f, z_hub));
			glm::mat4 renderHubM = glm::scale(hubM, glm::vec3(0.3f, 0.3f, 0.7f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderHubM));
			meshList[0]->RenderMesh();

			glm::mat4 llantaM = glm::translate(modelaux2, glm::vec3(0.0f, -2.5f, z_llanta));
			llantaM = glm::rotate(llantaM, glm::radians(rotRuedas[idxBase + 1]), glm::vec3(0.0f, 0.0f, 1.0f));
			llantaM = glm::rotate(llantaM, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 renderLlantaM = glm::scale(llantaM, glm::vec3(1.5f, 1.0f, 1.5f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderLlantaM));
			glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(0.15f, 0.15f, 0.15f)));
			meshList[2]->RenderMeshGeometry();


			// ---------------- 3. RUEDA TRASERA ----------------
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));

			glm::mat4 pegT = glm::translate(modelaux2, glm::vec3(3.0f, -0.75f, z_peg));
			glm::mat4 renderPegT = glm::scale(pegT, glm::vec3(0.3f, 0.3f, 0.5f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderPegT));
			meshList[0]->RenderMesh();

			glm::mat4 horizT = glm::translate(modelaux2, glm::vec3(3.25f, -0.75f, z_brazo));
			glm::mat4 renderHorizT = glm::scale(horizT, glm::vec3(0.5f, 0.3f, 0.3f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderHorizT));
			meshList[0]->RenderMesh();

			glm::mat4 vertT = glm::translate(modelaux2, glm::vec3(3.75f, -1.625f, z_brazo));
			vertT = glm::rotate(vertT, glm::radians(-74.05f), glm::vec3(0.0f, 0.0f, 1.0f));
			glm::mat4 renderVertT = glm::scale(vertT, glm::vec3(1.82f, 0.3f, 0.3f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderVertT));
			meshList[0]->RenderMesh();

			glm::mat4 hubT = glm::translate(modelaux2, glm::vec3(4.0f, -2.5f, z_hub));
			glm::mat4 renderHubT = glm::scale(hubT, glm::vec3(0.3f, 0.3f, 0.7f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderHubT));
			meshList[0]->RenderMesh();

			glm::mat4 llantaT = glm::translate(modelaux2, glm::vec3(4.0f, -2.5f, z_llanta));
			llantaT = glm::rotate(llantaT, glm::radians(rotRuedas[idxBase + 2]), glm::vec3(0.0f, 0.0f, 1.0f));
			llantaT = glm::rotate(llantaT, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 renderLlantaT = glm::scale(llantaT, glm::vec3(1.5f, 1.0f, 1.5f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderLlantaT));
			glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(0.15f, 0.15f, 0.15f)));
			meshList[2]->RenderMeshGeometry();
		}


		// MODELO 2: SONDA VOYAGER 1
		glm::mat4 modelVoyager(1.0);
		modelVoyager = glm::translate(modelVoyager, glm::vec3(8.0f, 8.0f, -12.0f));
		modelVoyager = glm::rotate(modelVoyager, glm::radians(rotVoyager), glm::vec3(0.0f, 1.0f, 0.0f));
		modelVoyager = glm::rotate(modelVoyager, glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 1.0f));

		glm::mat4 cuerpoCentral = modelVoyager;

		// 1. CUERPO CENTRAL Y COMPONENTES
		// Base inferior 
		glm::mat4 renderCuerpo = glm::scale(cuerpoCentral, glm::vec3(2.0f, 1.0f, 2.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderCuerpo));
		color = glm::vec3(0.15f, 0.15f, 0.15f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[0]->RenderMesh();

		// Bus de electrónica 
		glm::mat4 busCentral = glm::translate(cuerpoCentral, glm::vec3(0.0f, 0.75f, 0.0f));
		glm::mat4 renderBus = glm::scale(busCentral, glm::vec3(1.5f, 0.5f, 1.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderBus));
		color = glm::vec3(0.8f, 0.7f, 0.2f); // Dorado
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[0]->RenderMesh();

		// Tanque de combustible 
		glm::mat4 tanque = glm::translate(cuerpoCentral, glm::vec3(0.0f, -0.6f, 0.0f));
		glm::mat4 renderTanque = glm::scale(tanque, glm::vec3(0.8f, 0.8f, 0.8f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderTanque));
		color = glm::vec3(0.4f, 0.4f, 0.4f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		// Disco de Oro 
		glm::mat4 discoOro = glm::translate(cuerpoCentral, glm::vec3(1.05f, 0.0f, 0.0f));
		discoOro = glm::rotate(discoOro, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 renderDisco = glm::scale(discoOro, glm::vec3(0.6f, 0.05f, 0.6f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderDisco));
		color = glm::vec3(1.0f, 0.85f, 0.0f); // Amarillo Oro Brillante
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();


		// 2. ANTENA PRINCIPAL (HGA)
		glm::mat4 baseAntena = glm::translate(cuerpoCentral, glm::vec3(0.0f, 1.25f, 0.0f));
		glm::mat4 renderPlato = glm::scale(baseAntena, glm::vec3(4.5f, 0.5f, 4.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderPlato));
		color = glm::vec3(0.9f, 0.9f, 0.9f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[3]->RenderMeshGeometry();

		glm::mat4 mastilAntena = glm::translate(baseAntena, glm::vec3(0.0f, 0.6f, 0.0f));
		glm::mat4 renderMastil = glm::scale(mastilAntena, glm::vec3(0.15f, 1.5f, 0.15f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderMastil));
		color = glm::vec3(0.3f, 0.3f, 0.3f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();

		// Sub-reflector 
		glm::mat4 subReflector = glm::translate(mastilAntena, glm::vec3(0.0f, 0.7f, 0.0f));
		glm::mat4 renderSubRef = glm::scale(subReflector, glm::vec3(0.3f, 0.3f, 0.3f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderSubRef));
		color = glm::vec3(0.9f, 0.9f, 0.9f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();


		// 3. BRAZO DE ENERGÍA (RTG BOOM) 
		glm::mat4 brazoRTG = glm::translate(cuerpoCentral, glm::vec3(-1.0f, -0.2f, 0.0f));
		brazoRTG = glm::rotate(brazoRTG, glm::radians(25.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		brazoRTG = glm::translate(brazoRTG, glm::vec3(-2.5f, 0.0f, 0.0f));

		glm::mat4 renderBrazoRTG = glm::scale(brazoRTG, glm::vec3(5.0f, 0.15f, 0.15f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderBrazoRTG));
		color = glm::vec3(0.6f, 0.6f, 0.6f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[0]->RenderMesh();

		glm::mat4 baseGeneradores = glm::translate(brazoRTG, glm::vec3(-1.5f, 0.0f, 0.0f));
		baseGeneradores = glm::rotate(baseGeneradores, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		for (int i = -1; i <= 1; i++) {
			glm::mat4 rtg = glm::translate(baseGeneradores, glm::vec3(0.0f, i * 0.7f, 0.0f));
			glm::mat4 renderRTG = glm::scale(rtg, glm::vec3(0.6f, 0.6f, 0.6f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderRTG));
			color = glm::vec3(0.1f, 0.1f, 0.1f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			meshList[2]->RenderMeshGeometry();
		}


		// 4. BRAZO CIENTÍFICO (SCAN PLATFORM) 
		glm::mat4 brazoCientifico = glm::translate(cuerpoCentral, glm::vec3(1.0f, 0.5f, 0.0f));
		brazoCientifico = glm::rotate(brazoCientifico, glm::radians(-15.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		brazoCientifico = glm::translate(brazoCientifico, glm::vec3(2.5f, 0.0f, 0.0f));

		glm::mat4 renderBrazoCien = glm::scale(brazoCientifico, glm::vec3(5.0f, 0.15f, 0.15f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderBrazoCien));
		color = glm::vec3(0.6f, 0.6f, 0.6f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[0]->RenderMesh();

		glm::mat4 plataforma = glm::translate(brazoCientifico, glm::vec3(2.5f, -0.5f, 0.0f));
		glm::mat4 renderPlat = glm::scale(plataforma, glm::vec3(1.2f, 1.0f, 1.2f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderPlat));
		color = glm::vec3(0.8f, 0.7f, 0.2f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[0]->RenderMesh();

		glm::mat4 lente = glm::translate(plataforma, glm::vec3(0.0f, 0.2f, 0.6f));
		glm::mat4 renderLente = glm::scale(lente, glm::vec3(0.4f, 0.4f, 0.8f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderLente));
		color = glm::vec3(0.2f, 0.2f, 0.2f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[2]->RenderMeshGeometry();

		// Sensor secundario celeste
		glm::mat4 sensorCeleste = glm::translate(plataforma, glm::vec3(0.3f, 0.5f, -0.2f));
		glm::mat4 renderSensCel = glm::scale(sensorCeleste, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderSensCel));
		color = glm::vec3(0.2f, 0.6f, 0.8f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();


		// 5. BRAZO DEL MAGNETÓMETRO 
		glm::mat4 brazoMag = glm::translate(cuerpoCentral, glm::vec3(0.0f, 0.0f, 1.0f));
		brazoMag = glm::rotate(brazoMag, glm::radians(35.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		brazoMag = glm::rotate(brazoMag, glm::radians(20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		brazoMag = glm::translate(brazoMag, glm::vec3(0.0f, 0.0f, 6.0f));

		glm::mat4 renderBrazoMag = glm::scale(brazoMag, glm::vec3(0.08f, 0.08f, 12.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderBrazoMag));
		color = glm::vec3(0.7f, 0.7f, 0.7f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[0]->RenderMesh();

		glm::mat4 sensorMag = glm::translate(brazoMag, glm::vec3(0.0f, 0.0f, 6.0f));
		glm::mat4 renderSensor = glm::scale(sensorMag, glm::vec3(0.3f, 0.3f, 0.3f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(renderSensor));
		color = glm::vec3(0.9f, 0.8f, 0.2f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		glUseProgram(0);
		mainWindow.swapBuffers();
	}
	return 0;
}