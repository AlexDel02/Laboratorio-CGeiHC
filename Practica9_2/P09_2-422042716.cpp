/*
Practica 9-2: Animacion compleja
*/
//para cargar imagen
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <algorithm>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Window.h"
#include "Mesh_tn.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Model.h"
#include "Skybox.h"

//para iluminacion
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

#include "AnimacionCompleja.h"
#include "Humo.h"         // sistema de particulas

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Shader> shaderList;
Shader* shaderHumo;                 // shader propio del humo (shaders/humo.vert y humo.frag)
MeshModel* cuadroHumo;              // un cuadrado que se repite para cada particula

Camera camera;
Texture humoTexture;

Model cuerpo, interior, cristal;
Model esquina0;
Model esquina1;
Model esquina2;
Model esquina3;
Model esquina4;
Model esquina5;
Model esquina6;
Model esquina7;
Model cohete0;
Model cohete1;
Model cohete2;
Model cohete3;
Texture blanco;
MeshModel plano;
Skybox skybox;
Material Material_avion;
Material Material_opaco;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

// Tiempo de simulacion compartido: P pausa y R reinicia ambos objetos.
double tiempoAnimacion = 0.0;
SistemaHumo humo(1000);

bool pausa = false, anteriorP = false, anteriorR = false;

// Vertex Shader
static const char* vShader = "shaders/shader_light.vert";
// Fragment Shader
static const char* fShader = "shaders/shader_light.frag";

void CrearCuadroHumo()
{
	unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };
	GLfloat vertices[] = {
		//	x      y     z		u     v		nx    ny    nz
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,	0.0f, 0.0f, 1.0f,
		 0.5f, -0.5f, 0.0f,		1.0f, 0.0f,	0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f, 0.0f,		1.0f, 1.0f,	0.0f, 0.0f, 1.0f,
		-0.5f,  0.5f, 0.0f,		0.0f, 1.0f,	0.0f, 0.0f, 1.0f
	};
	cuadroHumo = new MeshModel();
	cuadroHumo->CreateMeshModel(vertices, indices, 32, 6);
}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);

	shaderHumo = new Shader();
	shaderHumo->CreateFromFiles("shaders/humo.vert", "shaders/humo.frag");
}

void DibujarModelo(Model& objeto, const glm::mat4& matriz,
	const glm::vec3& color, GLuint uniformModel, GLuint uniformColor)
{
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(matriz));
	glUniform3f(uniformColor, color.x, color.y, color.z);
	// Se enlaza explicitamente: el Model.cpp original no aplica correctamente
	// la textura cuando el archivo tiene un unico material de indice cero.
	blanco.UseTexture();
	objeto.RenderModel();
}

void CrearPlano()
{
	GLfloat vertices[] = {
		-100,0,-100, 0,0, 0,-1,0,
		-100,0, 100, 0,1, 0,-1,0,
		 100,0, 100, 1,1, 0,-1,0,
		 100,0,-100, 1,0, 0,-1,0
	};
	unsigned int indices[] = { 0,1,2,0,2,3 };
	plano.CreateMeshModel(vertices, indices, 32, 6);
}

int main()
{
	mainWindow = Window(1366, 768);
	if (mainWindow.Initialise() != 0) return 1;

	CreateShaders();
	CrearCuadroHumo();
	CrearPlano();
	glfwSetWindowTitle(glfwGetCurrentContext(), "Practica 9-2 Animacion Compleja");

	camera = Camera(glm::vec3(20.0f, 30.0f, 95.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -12.0f, 40.0f, 0.15f);

	humoTexture = Texture("Textures/humo.png");
	humoTexture.LoadTextureA();

	blanco = Texture("Textures/plain.png");
	blanco.LoadTextureA();
	cuerpo.LoadModel("Models/Holocron_Cuerpo.obj");
	interior.LoadModel("Models/Holocron_Interior.obj");
	cristal.LoadModel("Models/Holocron_Cristal.obj");
	esquina0.LoadModel("Models/Holocron_Esquina0.obj");
	esquina1.LoadModel("Models/Holocron_Esquina1.obj");
	esquina2.LoadModel("Models/Holocron_Esquina2.obj");
	esquina3.LoadModel("Models/Holocron_Esquina3.obj");
	esquina4.LoadModel("Models/Holocron_Esquina4.obj");
	esquina5.LoadModel("Models/Holocron_Esquina5.obj");
	esquina6.LoadModel("Models/Holocron_Esquina6.obj");
	esquina7.LoadModel("Models/Holocron_Esquina7.obj");
	cohete0.LoadModel("Models/Cohete_Parte0.obj");
	cohete1.LoadModel("Models/Cohete_Parte1.obj");
	cohete2.LoadModel("Models/Cohete_Parte2.obj");
	cohete3.LoadModel("Models/Cohete_Parte3.obj");

	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");
	skybox = Skybox(skyboxFaces);

	Material_avion = Material(1.0f, 64);
	Material_opaco = Material(0.3f, 4);

	// Solo luz direccional (sol); no se usan luces puntuales ni reflectores
	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
		0.55f, 0.6f,
		-0.3f, -1.0f, -0.4f);

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformColor = 0;
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	// Ubicacion de las variables del shader del humo (se consultan una sola vez)
	shaderHumo->UseShader();
	GLint programaHumo = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programaHumo);
	GLint hProjection = glGetUniformLocation(programaHumo, "projection");
	GLint hView = glGetUniformLocation(programaHumo, "view");
	GLint hModel = glGetUniformLocation(programaHumo, "model");
	GLint hColor = glGetUniformLocation(programaHumo, "color");
	GLint hOpacidad = glGetUniformLocation(programaHumo, "opacidad");
	glUseProgram(0);

	glm::mat4 model(1.0);
	lastTime = static_cast<float>(glfwGetTime());
	printf("P: pausa | R: reiniciar | W A S D y mouse: camara\n");

	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = (GLfloat)glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;
		// Se conserva el tiempo real para que la animacion no dependa de los FPS.
		glfwPollEvents();

		if (mainWindow.getsKeys()[GLFW_KEY_P] && !anteriorP) pausa = !pausa;
		anteriorP = mainWindow.getsKeys()[GLFW_KEY_P];
		if (mainWindow.getsKeys()[GLFW_KEY_R] && !anteriorR) { tiempoAnimacion = 0.0; humo.Reiniciar(); }
		anteriorR = mainWindow.getsKeys()[GLFW_KEY_R];
		float paso = pausa ? 0.0f : deltaTime;

		// ANIMACION
		tiempoAnimacion += paso;
		p92::Vuelo vuelo = p92::Cohete(tiempoAnimacion);
		glm::mat4 matrizCohete = p92::MatrizCohete(vuelo);
		float restanteHumo = paso;
		while (restanteHumo > 0.0f)
		{
			float subpaso = glm::min(restanteHumo, 0.02f);
			humo.Actualizar(subpaso);
			restanteHumo -= subpaso;
		}
		if (tiempoAnimacion > 2.0)
		{
			glm::vec3 escape = -glm::normalize(vuelo.velocidad);
			// El origen normalizado del modelo es su base.
			humo.Emitir(0, vuelo.posicion, escape, glm::min(paso, 0.05f));
		}

		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// DIBUJO
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformColor = shaderList[0].getColorLocation();
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, 0);
		shaderList[0].SetSpotLights(spotLights, 0);
		glUniform3f(uniformColor, 1.0f, 1.0f, 1.0f);


		// Plano de referencia y material general.
		Material_avion.UseMaterial(uniformSpecularIntensity, uniformShininess);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(0, -0.05f, 0));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3f(uniformColor, 0.18f, 0.22f, 0.28f);
		blanco.UseTexture();
		plano.RenderMeshModel();

		// Nucleo
		model = p92::Holocron(tiempoAnimacion);
		DibujarModelo(cuerpo, model, glm::vec3(1.0f), uniformModel, uniformColor);
		DibujarModelo(interior, model, glm::vec3(1.0f), uniformModel, uniformColor);
		DibujarModelo(cristal, model, glm::vec3(1.0f), uniformModel, uniformColor);

		// Ocho llamadas explicitas: una matriz independiente por esquina.
		model = p92::Esquina(tiempoAnimacion, glm::vec3(-2.608557f, -2.582415f, -2.678393f), glm::vec3(-1, -1, -1));
		DibujarModelo(esquina0, model, glm::vec3(1.0f), uniformModel, uniformColor);
		model = p92::Esquina(tiempoAnimacion, glm::vec3(-2.623192f, -2.692147f, 2.552887f), glm::vec3(-1, -1, 1));
		DibujarModelo(esquina1, model, glm::vec3(1.0f), uniformModel, uniformColor);
		model = p92::Esquina(tiempoAnimacion, glm::vec3(-2.613219f, 2.698192f, -2.546666f), glm::vec3(-1, 1, -1));
		DibujarModelo(esquina2, model, glm::vec3(1.0f), uniformModel, uniformColor);
		model = p92::Esquina(tiempoAnimacion, glm::vec3(-2.610460f, 2.561737f, 2.683653f), glm::vec3(-1, 1, 1));
		DibujarModelo(esquina3, model, glm::vec3(1.0f), uniformModel, uniformColor);
		model = p92::Esquina(tiempoAnimacion, glm::vec3(2.612639f, -2.558692f, -2.672239f), glm::vec3(1, -1, -1));
		DibujarModelo(esquina4, model, glm::vec3(1.0f), uniformModel, uniformColor);
		model = p92::Esquina(tiempoAnimacion, glm::vec3(2.599117f, -2.695634f, 2.519551f), glm::vec3(1, -1, 1));
		DibujarModelo(esquina5, model, glm::vec3(1.0f), uniformModel, uniformColor);
		model = p92::Esquina(tiempoAnimacion, glm::vec3(2.615375f, 2.708559f, -2.527707f), glm::vec3(1, 1, -1));
		DibujarModelo(esquina6, model, glm::vec3(1.0f), uniformModel, uniformColor);
		model = p92::Esquina(tiempoAnimacion, glm::vec3(2.578276f, 2.579881f, 2.677122f), glm::vec3(1, 1, 1));
		DibujarModelo(esquina7, model, glm::vec3(1.0f), uniformModel, uniformColor);

		// Todas las partes del cohete comparten posicion y orientacion.
		DibujarModelo(cohete0, matrizCohete, glm::vec3(0.95f, 0.95f, 0.95f), uniformModel, uniformColor);
		DibujarModelo(cohete1, matrizCohete, glm::vec3(0.12f, 0.12f, 0.12f), uniformModel, uniformColor);
		DibujarModelo(cohete2, matrizCohete, glm::vec3(0.67f, 0.67f, 0.67f), uniformModel, uniformColor);
		DibujarModelo(cohete3, matrizCohete, glm::vec3(0.60f, 0.0f, 0.0f), uniformModel, uniformColor);

		// Humo: se dibuja al final, con transparencia y sin escribir profundidad
		shaderHumo->UseShader();
		glUniformMatrix4fv(hProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(hView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);
		humoTexture.UseTexture();
		humo.Dibujar(cuadroHumo, camera.calculateViewMatrix(), camera.getCameraPosition(), hModel, hColor, hOpacidad);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);

		glUseProgram(0);
		mainWindow.swapBuffers();
	}

	return 0;
}
